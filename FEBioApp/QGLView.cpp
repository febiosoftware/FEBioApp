#include "stdafx.h"
#include <glew.h>
#include "QGLView.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <QMouseEvent>
#include <QMessageBox>
#include <QMenu>
#include <QPainter>
#include <GLWLib/GLWidgetManager.h>
#include <FSCore/color.h>
#include <MathLib/math3d.h>
#include <FEBioAppLib/FEBioData.h>

//-----------------------------------------------------------------------------
QGLView::QGLView(QWidget* parent, int w, int h) : QOpenGLWidget(parent)
{
	if (w < 200) w = 200;
	if (h < 200) h = 200;
	m_sizeHint = QSize(w, h);

	m_sztime[0] = 0;
	m_timeFormat = 0;

	m_smoothingAngle = 60;

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

	m_userRange = false;
	m_rng[0] = m_rng[1] = 0;

	Post::ColorMapManager::Initialize();

	m_col = new Post::CColorTexture;

	m_mesh = nullptr;

	CGLWidgetManager* wm = CGLWidgetManager::GetInstance();
	wm->AttachToView(this);
	wm->AddWidget(m_triad = new GLTriad(0, 0, 150, 150));
	m_triad->align(GLW_ALIGN_LEFT | GLW_ALIGN_BOTTOM);
	m_triad->show_coord_labels(true);

	wm->AddWidget(m_time = new GLBox(0, 0, 200, 30, ""));
	m_time->align(GLW_ALIGN_LEFT | GLW_ALIGN_TOP);

	wm->AddWidget(m_legend = new GLLegendBar(m_col, 0, 0, 120, 400));
	m_legend->align(GLW_ALIGN_RIGHT | GLW_ALIGN_VCENTER);

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
void QGLView::SetForegroundColor(double r, double g, double b)
{
	GLColor c((byte)(255.0*r), (byte)(255.0*g), (byte)(255.0*b));
	m_triad->set_fg_color(c);
	m_time->set_fg_color(c);
	m_legend->set_fg_color(c);
}

//-----------------------------------------------------------------------------
void QGLView::SetTimeFormat(int nformat)
{
	m_timeFormat = nformat;
}

//-----------------------------------------------------------------------------
void QGLView::SetFEModel(FEBioData* feb)
{
	m_feb = feb;

	// rebuild the FE surface
	if (m_mesh) delete m_mesh;
	m_mesh = m_feb->BuildGLMesh();

	// partition the surface
	m_mesh->PartitionFaces(m_smoothingAngle);

	Update();
}

//-----------------------------------------------------------------------------
void QGLView::SetDataSource(const char* szdata)
{
	m_map = szdata;
}

//-----------------------------------------------------------------------------
void QGLView::SetDataRange(double vmin, double vmax)
{
	m_userRange = true;
	m_rng[0] = vmin;
	m_rng[1] = vmax;
}

//-----------------------------------------------------------------------------
void QGLView::SetRotation(double eulerX, double eulerY, double eulerZ)
{
	const double D2R = 3.1415926 / 180.0;
	quatd q;
	q.SetEuler(eulerX*D2R, eulerY*D2R, eulerZ*D2R);
	m_cam.SetOrientation(q);

	m_cam.Update(true);
}

//-----------------------------------------------------------------------------
void QGLView::SetSmoothingAngle(double w)
{
	m_smoothingAngle = w;
}

//-----------------------------------------------------------------------------
void QGLView::Update(bool bzoom)
{
	// find the center of the box
	if (m_mesh == 0) return;

	if (bzoom)
	{
		FEBioApp::GLMesh::POINT p = MeshCenter(*m_mesh);
		m_cam.SetTarget(vec3d(p.x, p.y, p.z));
		double D = MeshSize(*m_mesh)*1.5;
		m_cam.SetTargetDistance(D);
		m_zmax = 2*D;
		m_zmin = 1e-4*D;
	}

	// update the gl mesh
	m_feb->UpdateGLMesh(m_mesh, m_map);

	m_feb->GetDataRange(m_rng);
	if (m_legend) m_legend->SetRange(m_rng[0], m_rng[1]);

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
}

void QGLView::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	int w = width();
	int h = height();
	if (h == 0) h = 1;
	double ar = (double)w / (double)h;

	gluPerspective(60.0, ar, m_zmin, m_zmax);
	glMatrixMode(GL_MODELVIEW);

	m_cam.Transform();

	if (m_mesh==0) return;

	glColor3ub(236, 212, 212);
	glEnable(GL_TEXTURE_1D);
	glEnable(GL_DEPTH_TEST);
	m_col->GetTexture().MakeCurrent();
	m_mesh->Render();

	glUseProgram(0);
	if (m_bshader) glUseProgram(myProgram);

	// set the projection Matrix to ortho2d so we can draw some stuff on the screen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width(), height(), 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_TEXTURE_1D);

	if (m_feb)
	{
		double time = m_feb->GetSimulationTime();

		if (m_timeFormat == 1)
		{
			int nsec = (int) fmod(time, 60.0); time /= 60.0;
			int nmin = (int) fmod(time, 60.0); time /= 60.0;
			int nhr  = (int) fmod(time, 24.0); time /= 24.0;
			int days = (int) time;
			sprintf(m_sztime, " Time = %d:%02d:%02d:%02d", days, nhr, nmin, nsec);
		}
		else
		{
			sprintf(m_sztime, " Time = %.4g", time);
		}
	}
	else m_sztime[0] = 0;
	m_time->set_label(m_sztime);

	// render the widgets
	m_triad->setOrientation(m_cam.GetOrientation());
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	CGLWidgetManager* wm = CGLWidgetManager::GetInstance();
	wm->DrawWidgets(&painter);
	painter.end();
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
