#pragma once
#include <QOpenGLWidget>
#include <FECore/vec3d.h>
#include <QAction>
#include "GLMesh.h"
#include "GLCamera.h"
#include <GLWLib/GLWidget.h>

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

	void SetBackgroundColor(double r, double g, double b);
	void SetForegroundColor(double r, double g, double b);

	void Update(bool bzoom = true);

	void SetDataSource(const char* szdata);

	void SetDataRange(double vmin, double vmax);

	void SetRotation(double eulerX, double eulerY, double eulerZ);

	void SetSmoothingAngle(double w);

	void SetTimeFormat(int nformat);

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

	std::string		m_map;

	GLBox*			m_time;
	GLTriad*		m_triad;
	GLLegendBar*	m_legend;
	CColorTexture*	m_col;

	double	m_smoothingAngle;

	double m_bgcol[3];

	bool	m_userRange;
	double	m_rng[2];

	char	m_sztime[256];
	int		m_timeFormat;

	QAction*	m_pShader;
};
