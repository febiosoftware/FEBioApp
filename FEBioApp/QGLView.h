#pragma once
#include <QOpenGLWidget>
#include <FECore/vec3d.h>
#include "GLMesh.h"

//-----------------------------------------------------------------------------
class FEModel;
class FESurface;

//-----------------------------------------------------------------------------
class QGLView : public QOpenGLWidget
{
//	Q_OBJECT

public:
	QGLView(QWidget* parent = 0, int w = 0, int h = 0);
	~QGLView();

	QSize minimumSizeHint() const { return QSize(200, 200); }
	QSize sizeHint() const { return m_sizeHint; }

	void SetFEModel(FEModel* pfem);

	void Update();

protected:
	void mousePressEvent  (QMouseEvent* ev);
	void mouseMoveEvent   (QMouseEvent* ev);
	void mouseReleaseEvent(QMouseEvent* ev);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	FEModel*	m_pfem;
	FESurface*	m_psurf;
	GLMesh		m_glmesh;
	vec3d		m_center;
	double		m_dist;
	QPoint		m_mousePos;
	double		m_xangle, m_zangle;
	QSize		m_sizeHint;

	GLuint	myVertexShader;
	GLuint	myFragmentShader;
	GLuint	myProgram;
};
