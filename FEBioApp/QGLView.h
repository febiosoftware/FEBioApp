#pragma once
#include <QOpenGLWidget>
#include <FECore/vec3d.h>
#include <QAction>
#include "GLMesh.h"
#include "GLCamera.h"

//-----------------------------------------------------------------------------
class FEModel;
class FESurface;

//-----------------------------------------------------------------------------
class QGLView : public QOpenGLWidget
{
	Q_OBJECT

public:
	QGLView(QWidget* parent = 0, int w = 0, int h = 0);
	~QGLView();

	QSize minimumSizeHint() const { return QSize(200, 200); }
	QSize sizeHint() const { return m_sizeHint; }

	void SetFEModel(FEModel* pfem);

	void Update(bool bzoom = true);

protected:
	void mousePressEvent  (QMouseEvent* ev);
	void mouseMoveEvent   (QMouseEvent* ev);
	void mouseReleaseEvent(QMouseEvent* ev);
	void contextMenuEvent (QContextMenuEvent* ev);
	bool event(QEvent* event);

public slots:
	void OnActivateShader();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	void initShaders();
	void initTextures();
	void drawTriad();

private:
	FEModel*	m_pfem;
	FESurface*	m_psurf;
	GLMesh		m_glmesh;
	QPoint		m_mousePos;

	double		m_zmin, m_zmax;
	QSize		m_sizeHint;

	CGLCamera	m_cam;

	GLuint	myVertexShader;
	GLuint	myFragmentShader;
	GLuint	myProgram;
	bool	m_bshader;

	QAction*	m_pShader;
};
