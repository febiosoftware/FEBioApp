#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <FECore/vec3d.h>

//-----------------------------------------------------------------------------
class FEModel;
class FESurface;

//-----------------------------------------------------------------------------
class QGLView : public QOpenGLWidget//, public QOpenGLFunctions
{
//	Q_OBJECT

public:
	QGLView(QWidget* parent = 0);

	QSize minimumSizeHint() const { return QSize(400, 400); }
	QSize sizeHint() const { return minimumSizeHint(); }

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
	vec3d		m_center;
	double		m_dist;
	QPoint		m_mousePos;
	double		m_xangle, m_zangle;
};
