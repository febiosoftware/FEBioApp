#pragma once
#include <QWidget>
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
class QPainter;
class QAction;

//-----------------------------------------------------------------------------
// manages a set of (x,y) value pairs
class QPlotData
{
public:
	QPlotData();
	QPlotData(const QPlotData& d);
	QPlotData& operator = (const QPlotData& d);

	//! clear data
	void clear();

	// add a point to the data
	void addPoint(double x, double y);

	// number of points
	int size() const { return (int) m_data.size(); }

	// get a data point
	QPointF& Point(int i) { return m_data[i]; }

	// get the bounding rectangle
	QRectF boundRect() const;

protected:
	vector<QPointF>	m_data;
};

//-----------------------------------------------------------------------------
//! This class implements a plotting widget. 
class QPlotWidget : public QWidget
{
	Q_OBJECT

public:
	//! constructor
	QPlotWidget(QWidget* parent = 0, int w = 0, int h = 0);

	//! Set the plot title
	void setTitle(const QString& s);

	// size hint
	QSize sizeHint() const { return m_sizeHint; }
	QSize minimumSizeHint() const { return QSize(200, 200); }

	// clear plot data
	void clearData();

	// change the view so that it fits the data
	void fitToData();

	// add a data field
	void addPlotData(const QPlotData& p);

	// get a data field
	int plots() { return (int) m_data.size(); }
	QPlotData& getPlotData(int i) { return m_data[i]; }

protected:
	void mousePressEvent  (QMouseEvent* ev);
	void mouseMoveEvent   (QMouseEvent* ev);
	void mouseReleaseEvent(QMouseEvent* ev);
	void contextMenuEvent (QContextMenuEvent* ev);

public:
	QString	m_title;
	QRectF	m_viewRect;
	QRect	m_screenRect;
	QPoint	m_mousePos;
	double	m_xscale, m_yscale;

	QPointF ScreenToView(const QPoint& p);
	QPoint ViewToScreen(const QPointF& p);

private:
	//! render the plot
	void paintEvent(QPaintEvent* pe);

public slots:
	void OnZoomToFit();
	void OnShowProps();
	void OnCopyToClipboard();

private: // drawing helper functions
	void drawAxes(QPainter& p);
	void drawAllData(QPainter& p);
	void drawData(QPainter& p, QPlotData& data);
	void drawGrid(QPainter& p);
	void drawTitle(QPainter& p);

private:
	vector<QPlotData>	m_data;
	int			m_ncol;

private:
	QAction*	m_pZoomToFit;
	QAction*	m_pShowProps;
	QAction*	m_pCopyToClip;
	QSize		m_sizeHint;
};
