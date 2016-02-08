#include "stdafx.h"
#include "QGLView.h"
#include <FECore/FEModel.h>
#include <FECore/FEBox.h>
#include <FECore/FEElemElemList.h>
#include <gl/GLU.h>
#include <QMouseEvent>

//-----------------------------------------------------------------------------
QGLView::QGLView(QWidget* parent) : QOpenGLWidget(parent)
{
	m_psurf = 0;
	m_yangle = 0.0;
	m_dist = 0.0;
}

//-----------------------------------------------------------------------------
void QGLView::Update()
{
	if ((m_psurf==0) && (m_pfem))
	{
		m_psurf = m_pfem->GetMesh().ElementBoundarySurface();
	}

	// find the center of the box
	if (m_psurf)
	{
		FEBox box(*m_psurf);
		m_center = box.center();
		m_dist = box.maxsize()*2;
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
	glTranslated(0.0, 0.0, -m_dist);
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
