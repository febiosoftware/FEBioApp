#include "stdafx.h"
#include <glew.h>
#include "QGLView.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <FECore/FEModel.h>
#include <FECore/FEBox.h>
#include <FECore/FEElemElemList.h>
#include <QMouseEvent>

//-----------------------------------------------------------------------------
QGLView::QGLView(QWidget* parent, int w, int h) : QOpenGLWidget(parent)
{
	m_psurf = 0;
	m_xangle = 0.0;
	m_zangle = 0.0;
	m_dist = 0.0;
	if (w < 200) w = 200;
	if (h < 200) h = 200;
	m_sizeHint = QSize(w, h);

	QSurfaceFormat f = format();
	f.setSamples(4);
	setFormat(f);

	myVertexShader = 0;
	myFragmentShader = 0;
	myProgram = 0;
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

	if (m_psurf) delete m_psurf;
	m_psurf = m_pfem->GetMesh().ElementBoundarySurface();
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

/*
char vertex_shader_source[] =
	"void main(void)"
	"{"
	"	vec4 pos = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"	gl_Position = pos;"
	"	vec3 N = normalize(gl_NormalMatrix * gl_Normal);"
	"	vec4 V = gl_ModelViewMatrix * gl_Vertex;"
	"	vec3 L = normalize(gl_LightSource[0].position - V.xyz);"
	"	vec3 H = normalize(L + vec3(0.0, 0.0, 1.0));"
	"	const float specularExp = 128.0;"
	"	float f = dot(N, L);"
	"	vec4 diffuse = gl_Color * vec4(max(0.0, f));"
	"	float h = max(0.0, dot(N, H));"
	"	vec4 spec = vec4(0.0);"
	"	if (f > 0.0)"
	"		spec = vec4(pow(h, specularExp));"
	"	gl_FrontColor = diffuse + spec;"
	"	vec3 ndc = pos.xyz / pos.w;"
	"	gl_FrontSecondaryColor = vec4((ndc*0.5) + 0.5, 1.0);"
	"}";
*/

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
"															   \n"
"	gl_FrontColor = gl_Color*vec4(max(0.0,NdotL));			   \n"
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
	GLuint myProgram = glCreateProgram();
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
    gluPerspective(45.0, w / (float) h, 0.01, 100.0);
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
	FESurface& s = *m_psurf;

	glColor3ub(196, 186, 186);
	vec3d r[FEElement::MAX_NODES];
	glBegin(GL_TRIANGLES);
	{
		int NF = s.Elements();
		for (int i=0; i<NF; ++i)
		{
			FESurfaceElement& el = s.Element(i);
			int nf = el.Nodes();
			for (int j=0; j<nf; ++j) r[j] = s.Node(el.m_lnode[j]).m_rt;

			vec3d nu = (r[1] - r[0])^(r[3] - r[0]);
			nu.unit();

			glNormal3d(nu.x, nu.y, nu.z);
			glVertex3d(r[0].x, r[0].y, r[0].z);
			glVertex3d(r[1].x, r[1].y, r[1].z);
			glVertex3d(r[3].x, r[3].y, r[3].z);

			nu = (r[3] - r[2])^(r[1] - r[2]);
			nu.unit();

			glNormal3d(nu.x, nu.y, nu.z);
			glVertex3d(r[1].x, r[1].y, r[1].z);
			glVertex3d(r[2].x, r[2].y, r[2].z);
			glVertex3d(r[3].x, r[3].y, r[3].z);
		}
	}
	glEnd();
}
