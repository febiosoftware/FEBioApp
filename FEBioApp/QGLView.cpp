#include "stdafx.h"
#include "QGLView.h"
#include <FECore/FEModel.h>
#include <FECore/FEElemElemList.h>
#include <gl/GLU.h>
#include <QMouseEvent>

//-----------------------------------------------------------------------------
QGLView::QGLView(QWidget* parent) : QOpenGLWidget(parent)
{
	m_psurf = 0;
	m_yangle = 0.0;
}

//-----------------------------------------------------------------------------
void QGLView::Update()
{
	if ((m_psurf==0) && (m_pfem))
	{
		FEMesh& mesh = m_pfem->GetMesh();

		FEElemElemList EEL;
		EEL.Create(&mesh);

		FESurface* ps = new FESurface(&mesh);
		m_psurf = ps;

		int NF = 0;
		int NE = mesh.Elements();
		for (int i=0; i<NE; ++i)
		{
			FEElement& el = mesh.Domain(0).ElementRef(i);
			int nf = mesh.Faces(el);
			for (int j=0; j<nf; ++j)
			{
				if (EEL.Neighbor(i, j) == 0) NF++;
			}
		}

		ps->create(NF);
		NF = 0;
		for (int i=0; i<NE; ++i)
		{
			FEElement& el = mesh.Domain(0).ElementRef(i);
			int nf = mesh.Faces(el);
			for (int j=0; j<nf; ++j)
			{
				if (EEL.Neighbor(i, j) == 0)
				{
					FESurfaceElement& se = ps->Element(NF++);
					se.SetType(FE_QUAD4G4);
					int nf[4];
					mesh.GetFace(el, j, nf);

					se.m_node[0] = nf[0];
					se.m_node[1] = nf[1];
					se.m_node[2] = nf[2];
					se.m_node[3] = nf[3];
				}
			}
		}

		ps->Init();
	}

	// find the center of the box
	if (m_psurf)
	{
		vec3d c = m_psurf->Node(0).m_rt;
		for (int i=1; i<m_psurf->Nodes(); ++i)
		{
			c += m_psurf->Node(i).m_rt;
		}
		c /= (double) m_psurf->Nodes();
		m_center = c;
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

	m_yangle += dx*1.0;

	m_mousePos = p;
	repaint();
}

//-----------------------------------------------------------------------------
void QGLView::mouseReleaseEvent(QMouseEvent* ev)
{

}

//-----------------------------------------------------------------------------
void QGLView::initializeGL()
{
//	initializeOpenGLFunctions();
    glClearColor(1.f, 1.f, 1.f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

    static GLfloat lightPosition[4] = { 0, 0, 10, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
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
	glTranslated(0.0, 0.0, -3.0);
	glRotatef(m_yangle, 1.f, 1.f, 0.f);
	glTranslated(-m_center.x, -m_center.y, -m_center.z);

	if (m_psurf==0) return;
	FESurface& s = *m_psurf;

	glColor3ub(255,0,0);
	vec3d r[FEElement::MAX_NODES];
	glBegin(GL_QUADS);
	{
		int NF = s.Elements();
		for (int i=0; i<NF; ++i)
		{
			FESurfaceElement& el = s.Element(i);
			int nf = el.Nodes();
			for (int j=0; j<nf; ++j) r[j] = s.Node(el.m_lnode[j]).m_rt;

			vec3d nu = (r[1] - r[0])^(r[2] - r[0]);
			nu.unit();

			glNormal3d(nu.x, nu.y, nu.z);
			glVertex3d(r[0].x, r[0].y, r[0].z);
			glVertex3d(r[1].x, r[1].y, r[1].z);
			glVertex3d(r[2].x, r[2].y, r[2].z);
			glVertex3d(r[3].x, r[3].y, r[3].z);
		}
	}
	glEnd();
}
