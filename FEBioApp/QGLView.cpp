#include "stdafx.h"
#include <glew.h>
#include "QGLView.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <FECore/FEModel.h>
#include <FECore/FEBox.h>
#include <FECore/FEElemElemList.h>
#include <QMouseEvent>
#include <QMessageBox>
#include <QMenu>

//-----------------------------------------------------------------------------
QGLView::QGLView(QWidget* parent, int w, int h) : QOpenGLWidget(parent)
{
	m_psurf = 0;
	m_xangle = 0.0;
	m_zangle = 0.0;
	m_dist = 0.0;
	m_zmin = 0.01;
	m_zmax = 100.0;
	if (w < 200) w = 200;
	if (h < 200) h = 200;
	m_sizeHint = QSize(w, h);

	QSurfaceFormat f = format();
	f.setSamples(8);
	setFormat(f);

	myVertexShader = 0;
	myFragmentShader = 0;
	myProgram = 0;
	m_bshader = true;

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
void QGLView::Update()
{
	// find the center of the box
	if (m_psurf)
	{
		FEBox box(*m_psurf);
		m_center = box.center();
		m_dist = box.maxsize()*1.5;
		m_zmax = 2*m_dist;
		m_zmin = 1e-4*m_zmax;

		// copy nodal coordinates
		int NF = m_glmesh.Faces();
		for (int i=0; i<NF; ++i)
		{
			GLMesh::FACE& f = m_glmesh.Face(i);
			m_glmesh.nodePosition(f.lnode[0]) = m_psurf->Node(f.nid[0]).m_rt;
			m_glmesh.nodePosition(f.lnode[1]) = m_psurf->Node(f.nid[1]).m_rt;
			m_glmesh.nodePosition(f.lnode[2]) = m_psurf->Node(f.nid[2]).m_rt;
		}

		// assign texture coordinates
		// first we need to find the largest displacement
		double Dmax = 0.0;
		for (int i=0; i<m_psurf->Nodes(); ++i)
		{
			FENode& ni = m_psurf->Node(i);
			double D = (ni.m_rt - ni.m_r0).norm();
			if (D > Dmax) Dmax = D;
		}

		// then, assign texture coordinates
		for (int i=0; i<NF; ++i)
		{
			GLMesh::FACE& f = m_glmesh.Face(i);
			m_glmesh.nodeTexCoord1D(f.lnode[0]) = (m_psurf->Node(f.nid[0]).m_rt - m_psurf->Node(f.nid[0]).m_r0).norm() / Dmax;
			m_glmesh.nodeTexCoord1D(f.lnode[1]) = (m_psurf->Node(f.nid[1]).m_rt - m_psurf->Node(f.nid[1]).m_r0).norm() / Dmax;
			m_glmesh.nodeTexCoord1D(f.lnode[2]) = (m_psurf->Node(f.nid[2]).m_rt - m_psurf->Node(f.nid[2]).m_r0).norm() / Dmax;
		}

		// recalculate normals
		m_glmesh.UpdateNormals();
	}
}

//-----------------------------------------------------------------------------
void QGLView::mousePressEvent(QMouseEvent* ev)
{
	m_mousePos = ev->pos();
}

//-----------------------------------------------------------------------------
void QGLView::mouseMoveEvent(QMouseEvent* ev)
{
	QPoint p = ev->pos();
	int dx = p.x() - m_mousePos.x();
	int dy = p.y() - m_mousePos.y();

	m_xangle += dy*1.0;
	m_zangle += dx*1.0;

	m_mousePos = p;
	repaint();
}

//-----------------------------------------------------------------------------
void QGLView::mouseReleaseEvent(QMouseEvent* ev)
{

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

    glClearColor(0.8f, 0.8f, 1.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

    static GLfloat lightPosition[4] = { 0, 0, 10, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	initShaders();

	initTextures();

	Update();
}

//-----------------------------------------------------------------------------
void QGLView::initTextures()
{
	GLubyte toonTable[3][3] = {
		{  0,   0, 255},
		{  0, 255,   0},
		{255,   0,   0},
	};

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 3, 0, GL_RGB, GL_UNSIGNED_BYTE, toonTable);

	glEnable(GL_TEXTURE_1D);
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
    gluPerspective(45.0, w / (float) h, m_zmin, m_zmax);
    glMatrixMode(GL_MODELVIEW);
}

void QGLView::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslated(0.0, 0.0, -m_dist);
	glRotatef(m_xangle, 1.f, 0.f, 0.f);
	glRotatef(m_zangle, 0.f, 0.f, 1.f);
	glTranslated(-m_center.x, -m_center.y, -m_center.z);

	if (m_psurf==0) return;

	glColor3ub(255, 255, 255);
	m_glmesh.Render();
}

//-----------------------------------------------------------------------------
void QGLView::contextMenuEvent(QContextMenuEvent* ev)
{
	QMenu menu;
	menu.addAction(m_pShader);
	menu.exec(ev->globalPos());
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

#include "moc_qglview.cpp"
