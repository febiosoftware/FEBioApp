#include "stdafx.h"
#include <glew.h>
#include "QGLView.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <FECore/FEModel.h>
#include <FECore/FEBox.h>
#include <FECore/FEElemElemList.h>
#include <FECore/FESurface.h>
#include <QMouseEvent>
#include <QMessageBox>
#include <QMenu>
#include <QPainter>

//-----------------------------------------------------------------------------
QGLView::QGLView(QWidget* parent, int w, int h) : QOpenGLWidget(parent)
{
	m_psurf = 0;
	if (w < 200) w = 200;
	if (h < 200) h = 200;
	m_sizeHint = QSize(w, h);

	m_cam.SetTargetDistance(5.f);
	m_cam.GetOrientation() = quatd(-1, vec3d(1,0,0));
	m_cam.Update(true);

	QSurfaceFormat f = format();
	f.setSamples(8);
	setFormat(f);

	m_bgcol[0] = 0.8;
	m_bgcol[1] = 0.8;
	m_bgcol[2] = 1.0;

	myVertexShader = 0;
	myFragmentShader = 0;
	myProgram = 0;
	m_bshader = false;

	m_pShader = new QAction("Activate shader", this);
	connect(m_pShader, SIGNAL(triggered()), this, SLOT(OnActivateShader()));
}

//-----------------------------------------------------------------------------
QGLView::~QGLView()
{
	makeCurrent();

	// Clean up
	if (myProgram != 0) { glUseProgram(0); glDeleteProgram(myProgram); }
	if (myVertexShader != 0) glDeleteShader(myFragmentShader);
	if (myFragmentShader != 0) glDeleteShader(myVertexShader);

	doneCurrent();
}

//-----------------------------------------------------------------------------
void QGLView::SetBackgroundColor(double r, double g, double b)
{
	m_bgcol[0] = r;
	m_bgcol[1] = g;
	m_bgcol[2] = b;
}

//-----------------------------------------------------------------------------
void QGLView::SetFEModel(FEModel* pfem)
{
	m_pfem = pfem;

	// rebuild the FE surface
	if (m_psurf) delete m_psurf;
	m_psurf = m_pfem->GetMesh().ElementBoundarySurface();

	// Create the GL mesh
	int NN = m_psurf->Nodes();
	int NE = m_psurf->Elements();

	int NF = 0;
	for (int i=0; i<NE; ++i)
	{
		FESurfaceElement& el = m_psurf->Element(i);
		if (el.Nodes() == 3) NF++; 
		else if (el.Nodes() == 4) NF += 2;
	}
	m_glmesh.Create(NF);

	// create the connectivity
	NF = 0;
	for (int i=0; i<NE; ++i)
	{
		FESurfaceElement& el = m_psurf->Element(i);
		GLMesh::FACE& f1 = m_glmesh.Face(NF++);
		f1.nid[0] = el.m_lnode[0];
		f1.nid[1] = el.m_lnode[1];
		f1.nid[2] = el.m_lnode[2];

		if (el.Nodes() == 4)
		{
			GLMesh::FACE& f2 = m_glmesh.Face(NF++);
			f2.nid[0] = el.m_lnode[2];
			f2.nid[1] = el.m_lnode[3];
			f2.nid[2] = el.m_lnode[0];
		}
	}

	// copy initial nodal coordinates
	for (int i=0; i<NF; ++i)
	{
		GLMesh::FACE& f = m_glmesh.Face(i);
		m_glmesh.nodePosition(f.lnode[0]) = m_psurf->Node(f.nid[0]).m_rt;
		m_glmesh.nodePosition(f.lnode[1]) = m_psurf->Node(f.nid[1]).m_rt;
		m_glmesh.nodePosition(f.lnode[2]) = m_psurf->Node(f.nid[2]).m_rt;
	}

	// find the face neighbors
	m_glmesh.UpdateFaces();

	// partition the surface
	m_glmesh.PartitionFaces();

	Update();
}

//-----------------------------------------------------------------------------
void QGLView::SetDataSource(const char* szdata)
{
	m_map = szdata;
}

//-----------------------------------------------------------------------------
void QGLView::Update(bool bzoom)
{
	// find the center of the box
	if (m_psurf == 0) return;

	if (bzoom)
	{
		FEBox box(*m_psurf);
		m_cam.SetTarget(box.center());
		double D = box.maxsize()*1.5;
		m_cam.SetTargetDistance(D);
		m_zmax = 2*D;
		m_zmin = 1e-4*D;
	}

	// copy nodal coordinates
	int NF = m_glmesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		GLMesh::FACE& f = m_glmesh.Face(i);
		m_glmesh.nodePosition(f.lnode[0]) = m_psurf->Node(f.nid[0]).m_rt;
		m_glmesh.nodePosition(f.lnode[1]) = m_psurf->Node(f.nid[1]).m_rt;
		m_glmesh.nodePosition(f.lnode[2]) = m_psurf->Node(f.nid[2]).m_rt;
	}

	int NN = m_psurf->Nodes();
	vector<double> val(NN, 0);

	if (m_map.empty() == false)
	{
		DOFS& dofs = m_pfem->GetDOFS();
		int nvar = dofs.GetVariableIndex(m_map.c_str());
		if (nvar >= 0)
		{
			int ndof = dofs.GetVariableSize(nvar);
			int dof0 = dofs.GetDOF(nvar, 0);

			// evaluate data values
			double Dmin = 1e99, Dmax = -1e99;
			for (int i=0; i<m_psurf->Nodes(); ++i)
			{
				FENode& ni = m_psurf->Node(i);

				double D = 0;
				if (ndof == 1)
				{
					D = ni.get(dof0);
				}
				else
				{
					for (int j=0; j<ndof; ++j)
					{
						double dn = ni.get(dof0 + j);
						D += dn*dn;
					}
				}

				if (D < Dmin) Dmin = D;
				else if (D > Dmax) Dmax = D;

				val[i] = D;
			}
			if (Dmax == Dmin) Dmax++;

			// normalize textture coordinates
			for (int i=0; i<NN; ++i) val[i] = (val[i] - Dmin) / (Dmax - Dmin);
		}
	}

	// then, assign texture coordinates
	for (int i = 0; i<NF; ++i)
	{
		GLMesh::FACE& f = m_glmesh.Face(i);
		m_glmesh.nodeTexCoord1D(f.lnode[0]) = val[f.nid[0]];
		m_glmesh.nodeTexCoord1D(f.lnode[1]) = val[f.nid[1]];
		m_glmesh.nodeTexCoord1D(f.lnode[2]) = val[f.nid[2]];
	}

	// recalculate normals
	m_glmesh.UpdateNormals();

	repaint();
}

//-----------------------------------------------------------------------------
void QGLView::mousePressEvent(QMouseEvent* ev)
{
	m_mousePos = ev->pos();
	ev->accept();
}

//-----------------------------------------------------------------------------
void QGLView::mouseMoveEvent(QMouseEvent* ev)
{
	int x = ev->x();
	int y = ev->y();

	int xp = m_mousePos.x();
	int yp = m_mousePos.y();

	Qt::KeyboardModifiers key = ev->modifiers();
	Qt::MouseButtons buttons = ev->buttons();
	bool but1 = (buttons & Qt::LeftButton);
	bool but2 = (buttons & Qt::MiddleButton);
	bool but3 = (buttons & Qt::RightButton);
	bool balt   = (key & Qt::AltModifier);
	bool bshift = (key & Qt::ShiftModifier);

	if (but1)
	{
		// see if alt-button is pressed
		if (balt)
		{
			// rotate in-plane
			quatd qz = quatd((y - yp)*0.01, vec3d(0, 0, 1));
			m_cam.Orbit(qz);
		}
		else
		{
			quatd qx = quatd((y - yp)*0.01, vec3d(1, 0, 0));
			quatd qy = quatd((x - xp)*0.01, vec3d(0, 1, 0));
			m_cam.Orbit(qx);
			m_cam.Orbit(qy);
		}
		repaint();
	}
	else if ((but2) || (but3 && balt))
	{
		vec3d r = vec3d(-(float)(x - xp), (float)(y - yp), 0.f);

		double h = (double) height(); if (h==0) h = 1;
		double w = (double) width();
		double ar = w/h;
		double hf = 2.0*m_zmin*tan(3.1415926/180.0*60.0*0.5);
		double wf = ar*hf;

		double sx = wf/width();
		double sy = hf/height();
		double sz = m_cam.GetTargetDistance()/m_zmin;

		r.x *= (float)(sz*sx);
		r.y *= (float)(sz*sy);
		
		m_cam.Truck(r);
		repaint();
	}
	else if (but3)
	{
		if (yp > y) m_cam.Zoom(0.95f);
		if (yp < y) m_cam.Zoom(1.0f/0.95f);
	
//		if (m_zoom > 8) m_zoom = 8;
//		if (m_zoom < 1) m_zoom = 1;

		repaint();
	}
	// store mouse position
	m_mousePos = ev->pos();
	m_cam.Update(true);
}

//-----------------------------------------------------------------------------
void QGLView::mouseReleaseEvent(QMouseEvent* ev)
{
	ev->accept();
}

char vertex_shader_source[] =
"void main(void)												\n"
"{																\n"
"	vec4 vpos = gl_Vertex;\n"
"	gl_Position = gl_ModelViewProjectionMatrix * vpos;		\n"
"																\n"
"	vec3 N = normalize(gl_NormalMatrix * gl_Normal);			\n"
"	vec4 V = gl_ModelViewMatrix * gl_Vertex;					\n"
"	vec3 L = normalize(gl_LightSource[0].position.xyz - V.xyz);\n"
"	vec3 H = normalize(L + vec3(0,0,1));					   \n"
"															   \n"
"	float NdotL = dot(N, L);								   \n"
"	float NdotH = 0.99*max(0.0, dot(N, H));								   \n"
"															   \n"
"	const float specExp = 128.0;							   \n"
"															   \n"
"	vec4 spec = vec4(0.0);									   \n"
"	if (NdotL > 0) spec = gl_Color*vec4(pow(NdotH, specExp));		   \n"
"	         \n"
"   vec4 diff = gl_Color*vec4(max(0.0,NdotL));                 \n"
"   vec4 backLight = vec4(0.0);"
"   if (NdotL > 0) backLight = vec4(.3,.3,.4,1.)*vec4(1.0 - NdotL); \n"
"	gl_FrontColor = backLight + diff;		                       \n"
"	gl_FrontSecondaryColor = spec;							   \n"
"}															   \n";

char fragment_shader_source[] =
	"void main(void)"
	"{"
	"	gl_FragColor = gl_Color + gl_SecondaryColor;"
	"}";

//-----------------------------------------------------------------------------
void QGLView::initializeGL()
{
//	initializeOpenGLFunctions();
	glewInit();

	glClearColor((float)m_bgcol[0], (float)m_bgcol[1], (float)m_bgcol[2], 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

    static GLfloat lightPosition[4] = { 0, 0, 10, 1.0 };
//    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	initShaders();

	initTextures();

	Update();
}

//-----------------------------------------------------------------------------
void QGLView::initTextures()
{
	GLubyte toonTable[5][3] = {
		{  0,   0, 255},
		{  0, 255, 255},
		{  0, 255,   0},
		{255, 255,   0},
		{255,   0,   0},
	};

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 5, 0, GL_RGB, GL_UNSIGNED_BYTE, toonTable);
}

//-----------------------------------------------------------------------------
void QGLView::initShaders()
{
	// create the shader objects
	GLuint myVertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (myVertexShader==0) printf("ERRORL: Failed creating vertex shader\n");
	GLuint myFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (myFragmentShader==0) printf("ERRORL: Failed creating fragment shader\n");

	// set the shader source code
	GLchar* myStringPtrs[1];
	myStringPtrs[0] = vertex_shader_source;
	glShaderSource(myVertexShader, 1, myStringPtrs, NULL);
	myStringPtrs[0] = fragment_shader_source;
	glShaderSource(myFragmentShader, 1, myStringPtrs, NULL);

	// compile the shaders
	GLint success;
	glCompileShader(myVertexShader);
	glGetShaderiv(myVertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(myVertexShader, 512, NULL, infoLog);
		fprintf(stderr, "ERROR in vertex shader compilation!\n");
		fprintf(stderr, "Info log: %s\n", infoLog);
	}

	glCompileShader(myFragmentShader);
	glGetShaderiv(myFragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(myFragmentShader, 512, NULL, infoLog);
		fprintf(stderr, "ERROR in vertex shader compilation!\n");
		fprintf(stderr, "Info log: %s\n", infoLog);
	}

	// Create the program and attach the shaders
	myProgram = glCreateProgram();
	glAttachShader(myProgram, myVertexShader);
	glAttachShader(myProgram, myFragmentShader);

	// link the program
	glLinkProgram(myProgram);
	glGetProgramiv(myProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(myProgram, 512, NULL, infoLog);
		fprintf(stderr, "ERROR in program linkage!\n");
		fprintf(stderr, "Info log: %s\n", infoLog);
	}

	// validate the program
	glValidateProgram(myProgram);
	glGetProgramiv(myProgram, GL_VALIDATE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(myProgram, 512, NULL, infoLog);
		fprintf(stderr, "ERROR in program validation!\n");
		fprintf(stderr, "Info log: %s\n", infoLog);
	}

	// use the shader
	glUseProgram(myProgram);
}

//-----------------------------------------------------------------------------
void QGLView::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	if (h==0) h = 1;
	double ar = (double) w / (double) h;

	gluPerspective(60.0, ar, m_zmin, m_zmax);
    glMatrixMode(GL_MODELVIEW);
}

void QGLView::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_cam.Transform();

	if (m_psurf==0) return;

	glColor3ub(236, 212, 212);
	glEnable(GL_TEXTURE_1D);
	m_glmesh.Render();

	glUseProgram(0);
	drawTriad();
	if (m_bshader) glUseProgram(myProgram);
/*
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	QPainter painter(this);
	painter.setPen(Qt::black);
	painter.setFont(QFont("Arial", 12));
	painter.drawText(10, 50, "Hello");
	painter.end();
*/
}

//-----------------------------------------------------------------------------
// Draws the coordinate axes triad.
void QGLView::drawTriad()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	GLfloat ones[] = {1.f, 1.f, 1.f, 1.f};
	GLfloat ambient[] = {0.0f,0.0f,0.0f,1.f};
	GLfloat specular[] = {0.5f,0.5f,0.5f,1};
	GLfloat emission[] = {0,0,0,1};
	GLfloat	light[] = {0, 0, -1, 0};

	// get the viewport so that we may restore it later
	int view[4];
	glGetIntegerv(GL_VIEWPORT, view);

	// set the new viewport in the lower-left corner
	const int w = 100;
	const int h = 100;
	int x0 = 0;
	int y0 = 0;
	int x1 = x0 + w;
	int y1 = y0 + h;
	if (x1 < x0) { x0 ^= x1; x1 ^= x0; x0 ^= x1; }
	if (y1 < y0) { y0 ^= y1; y1 ^= y0; y0 ^= y1; }
	glViewport(x0, y0, x1-x0, y1-y0);

	// setup the projection mode
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	float d = 1.2f;
	float ar = 1.f;
	if (h != 0) ar = fabs((float) w / (float) h);
	if (ar >= 1.f)	glOrtho(-d*ar, d*ar, -d, d, -2, 2); else glOrtho(-d, d, -d/ar, d/ar, -2, 2);

	// reset the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// clear depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	// store attributes
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
/*
	glLightfv(GL_LIGHT0, GL_POSITION, light);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ones);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ones);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 32);
*/
	quatd q = m_cam.GetOrientation();
	vec3d r = q.GetVector();
	float a = 180*q.GetAngle()/3.1415926;

	if ((a > 0) && (r.norm() > 0))
		glRotatef(a, r.x, r.y, r.z);	

	// create the cylinder object
//	glEnable(GL_LIGHTING);
	GLUquadricObj* pcyl = gluNewQuadric();

	const GLdouble r0 = .05;
	const GLdouble r1 = .15;

	// draw x-axis
	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	glColor3ub(255, 0, 0);
	gluCylinder(pcyl, r0, r0, .9, 5, 1);
	glTranslatef(0,0,.8f);
	gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	glPopMatrix();

	// draw y-axis
	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glColor3ub(0, 255, 0);
	gluCylinder(pcyl, r0, r0, .9, 5, 1);
	glTranslatef(0,0,.8f);
	gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	glPopMatrix();

	// draw z-axis
	glPushMatrix();
	glColor3ub(0, 0, 255);
	gluCylinder(pcyl, r0, r0, .9, 5, 1);
	glTranslatef(0,0,.8f);
	gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	glPopMatrix();

	// cleanup
	gluDeleteQuadric(pcyl);

	// restore project matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// restore modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// restore viewport
	glViewport(view[0], view[1], view[2], view[3]);

	// restore attributes
	glPopAttrib();

/*	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	// draw the coordinate labels
	painter.setPen(Qt::black);
	painter.setFont(QFont("Helvetica", 12));
	vec3d ex(1.0, 0.0, 0.0);
	vec3d ey(0.0, 1.0, 0.0);
	vec3d ez(0.0, 0.0, 1.0);
	q.RotateVector(ex);
	q.RotateVector(ey);
	q.RotateVector(ez);

	y0 = view[3] - y0;
	y1 = view[3] - y1;

	ex.x = x0 + (x1 - x0)*(ex.x + 1)*0.5; ex.y = y0 + (y1 - y0)*(ex.y + 1)*0.5;
	ey.x = x0 + (x1 - x0)*(ey.x + 1)*0.5; ey.y = y0 + (y1 - y0)*(ey.y + 1)*0.5;
	ez.x = x0 + (x1 - x0)*(ez.x + 1)*0.5; ez.y = y0 + (y1 - y0)*(ez.y + 1)*0.5;

	painter.drawText(ex.x, ex.y, "X");
	painter.drawText(ey.x, ey.y, "Y");
	painter.drawText(ez.x, ez.y, "Z");
	painter.end();
*/
}

//-----------------------------------------------------------------------------
void QGLView::contextMenuEvent(QContextMenuEvent* ev)
{
/*	QMenu menu;
	menu.addAction(m_pShader);
	menu.exec(ev->globalPos());
*/
}

//-----------------------------------------------------------------------------
void QGLView::OnActivateShader()
{
	makeCurrent();
	if (m_bshader) glUseProgram(0);
	else glUseProgram(myProgram);
	GLenum err = glGetError();
	fprintf(stderr, "glUseProgram error code: %d\n", err);
	m_bshader = !m_bshader;
	doneCurrent();
	repaint();
}

//-----------------------------------------------------------------------------
bool QGLView::event(QEvent* event)
{
	switch (event->type())
	{
	case QEvent::TouchBegin:
	case QEvent::TouchCancel:
	case QEvent::TouchEnd:
	case QEvent::TouchUpdate:
		event->accept();
		{
			QTouchEvent* te = static_cast<QTouchEvent*>(event);
			QList<QTouchEvent::TouchPoint> points = te->touchPoints();
			if (points.count() == 2)
			{
				QTouchEvent::TouchPoint p0 = points.first();
				QTouchEvent::TouchPoint p1 = points.last();
				QLineF line1(p0.startPos(), p1.startPos());
				QLineF line2(p0.pos(), p1.pos());
				double scale = line2.length() / line1.length();

				if (scale > 1.0) m_cam.Zoom(0.95f);
				else m_cam.Zoom(1.0f/0.95f);

				repaint();
			}
		}
		return true;
	}
	return QOpenGLWidget::event(event);
}
