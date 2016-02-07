#include "stdafx.h"
#include "QPlotWidget.h"
#include <QPainter>
#include <QFontDatabase>
#include <QMouseEvent>

//-----------------------------------------------------------------------------
QPlotData::QPlotData()
{

}

//-----------------------------------------------------------------------------
QPlotData::QPlotData(const QPlotData& d)
{
	m_data = d.m_data;
}

//-----------------------------------------------------------------------------
QPlotData& QPlotData::operator = (const QPlotData& d)
{
	m_data = d.m_data;
	return *this;
}

//-----------------------------------------------------------------------------
void QPlotData::clear()
{ 
	m_data.clear(); 
}

//-----------------------------------------------------------------------------
QRectF QPlotData::boundRect() const
{
	QRectF r(m_data[0].x(), m_data[0].y(), 0.0, 0.0);
	for (int i=1; i<(int)m_data.size(); ++i)
	{
		const QPointF& p = m_data[i];
		if (p.x() < r.left  ()) r.setLeft  (p.x());
		if (p.x() > r.right ()) r.setRight (p.x());
		if (p.y() > r.bottom()) r.setBottom(p.y());
		if (p.y() < r.top   ()) r.setTop   (p.y());
	}
	return r;
}

//-----------------------------------------------------------------------------
void QPlotData::addPoint(double x, double y)
{
	QPointF p(x, y);
	m_data.push_back(p);
}

//-----------------------------------------------------------------------------
QPlotWidget::QPlotWidget(QWidget* parent) : QWidget(parent)
{
	m_viewRect = QRectF(-0.1, -0.1, 2.5, 2.5);

	QPlotData d1;
	m_data.push_back(d1);
}

//-----------------------------------------------------------------------------
void QPlotWidget::setTitle(const QString& t)
{
	m_title = t;
}

//-----------------------------------------------------------------------------
void QPlotWidget::clear()
{
	for (int i=0; i<(int) m_data.size(); ++i) m_data[i].clear();
}

//-----------------------------------------------------------------------------
void QPlotWidget::fitToData()
{
	if (m_data.empty()) return;

	QRectF r = m_data[0].boundRect();
	for (int i=1; i<(int) m_data.size(); ++i)
	{
		QRectF ri = m_data[i].boundRect();
		r = ri.united(r);
	}

	m_viewRect = r;

	double dx = 0.05*m_viewRect.width();
	double dy = 0.05*m_viewRect.height();
	m_viewRect.adjust(-dx, -dy, dx, dy);
}

//-----------------------------------------------------------------------------
void QPlotWidget::mousePressEvent(QMouseEvent* ev)
{
	m_mousePos = ev->pos();
}

//-----------------------------------------------------------------------------
void QPlotWidget::mouseMoveEvent(QMouseEvent* ev)
{
	QPoint p = ev->pos();
	QPointF r0 = ScreenToView(m_mousePos);
	QPointF r1 = ScreenToView(p);
	m_viewRect.translate(r0.x() - r1.x(), r0.y() - r1.y());
	m_mousePos = p;
	repaint();
}

//-----------------------------------------------------------------------------
void QPlotWidget::mouseReleaseEvent(QMouseEvent* ev)
{

}

//-----------------------------------------------------------------------------
QPointF QPlotWidget::ScreenToView(const QPoint& p)
{
	qreal x = m_viewRect.left  () + (m_viewRect.width ()*(p.x() - m_screenRect.left())/(m_screenRect.width ()));
	qreal y = m_viewRect.bottom() + (m_viewRect.height()*(m_screenRect.top() - p.y() )/(m_screenRect.height()));
	return QPointF(x, y);
}

//-----------------------------------------------------------------------------
QPoint QPlotWidget::ViewToScreen(const QPointF& p)
{
	int x = m_screenRect.left() + (int)(m_screenRect.width ()*(p.x() - m_viewRect.left  ())/(m_viewRect.width ()));
	int y = m_screenRect.top () - (int)(m_screenRect.height()*(p.y() - m_viewRect.bottom())/(m_viewRect.height()));
	return QPoint(x, y);
}

//-----------------------------------------------------------------------------
void QPlotWidget::paintEvent(QPaintEvent* pe)
{
	// Process base event first
	QWidget::paintEvent(pe);

	// store the current rectangle
	m_screenRect = rect();
	if ((m_screenRect.width()==0)||
		(m_screenRect.height()==0)) return;

	// Create the painter class
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);

	// clear the background
	p.fillRect(m_screenRect, Qt::white);

	// render the title
	drawTitle(p);

	m_screenRect.adjust(50, 0, -60, -30);
	p.setBrush(Qt::NoBrush);
	p.drawRect(m_screenRect);

	// draw the grid
	drawGrid(p);

	// draw the grid axes
	drawAxes(p);

	// render the data
	p.setClipRect(m_screenRect);
	drawAllData(p);
}

//-----------------------------------------------------------------------------
void QPlotWidget::drawTitle(QPainter& p)
{
	QPen pen(Qt::black, 1);
	p.setPen(pen);
	QFont f("Times", 12, QFont::Bold);
	p.setFont(f);
	QFontMetrics fm(f);
	QRect titleRect = m_screenRect;
	titleRect.setHeight(fm.height() + 10);
	p.drawText(titleRect, Qt::AlignCenter, m_title);
	m_screenRect.setTop(titleRect.bottom());
}

//-----------------------------------------------------------------------------
void QPlotWidget::drawGrid(QPainter& p)
{
	char sz[256] = {0};
	QFont f("Arial", 10);
	QFontMetrics fm(f);
	p.setFont(f);
	p.setPen(QPen(Qt::black));

	int x0 = m_screenRect.left();
	int x1 = m_screenRect.right();
	int y0 = m_screenRect.top();
	int y1 = m_screenRect.bottom();

	double xscale = 0.5;
	double yscale = 0.5;

	p.setPen(QPen(Qt::black, 0.25));

	// determine the y-scale
	double gy = 1;
	int nydiv = (int) log10(yscale);
	if (nydiv != 0)
	{
		gy = pow(10.0, nydiv);
		sprintf(sz, "x 1e%03d", nydiv);
		p.drawText(x0-30, y0 - fm.height() + fm.descent(), QString(sz));
	}

	// determine the x-scale
	double gx = 1;
	int nxdiv = (int) log10(xscale);
	if (nxdiv != 0)
	{
		gx = pow(10.0, nxdiv);
		sprintf(sz, "x 1e%03d", nxdiv);
		p.drawText(x1+5, y1, QString(sz));
	}

	// draw the y-grid lines
	double fy = yscale*(int)(m_viewRect.top()/yscale);
	while (fy < m_viewRect.bottom())
	{
		int iy = ViewToScreen(QPointF(0.0, fy)).y();
		QPainterPath path;
		path.moveTo(x0, iy);
		path.lineTo(x1-1, iy);
		p.drawPath(path);
		fy += yscale;
	}

	// draw the x-grid lines
	double fx = xscale*(int)(m_viewRect.left()/xscale);
	while (fx < m_viewRect.right())
	{
		int ix = ViewToScreen(QPointF(fx, 0.0)).x();
		QPainterPath path;
		path.moveTo(ix, y0);
		path.lineTo(ix, y1-1);
		p.drawPath(path);
		fx += xscale;
	}

	// draw the y-labels
	fy = yscale*(int)(m_viewRect.top()/yscale);
	while (fy < m_viewRect.bottom())
	{
		int iy = ViewToScreen(QPointF(0.0, fy)).y();
		if (iy < y1)
		{
			double g = fy / gy;
			if (fabs(g) < 1e-7) g = 0;
			sprintf(sz, "%lg", g);
			QString s(sz);
			int w = p.fontMetrics().width(s);
			p.drawText(x0 -w - 5, iy + p.fontMetrics().height()/3, s);
//			fl_line(x0-3, iy, x0+3, iy);
		}
		fy += yscale;
	}

	// draw the x-labels
	fx = xscale*(int)(m_viewRect.left()/xscale);
	while (fx < m_viewRect.right())
	{
		int ix = ViewToScreen(QPointF(fx, 0.0)).x();
		if (ix > x0)
		{
			double g = fx / gx;
			if (fabs(g) < 1e-7) g = 0;
			sprintf(sz, "%lg", g);
			QString s(sz);
			int w = p.fontMetrics().width(s);
			p.drawText(ix-w/2, y1+p.fontMetrics().height(), s);
//			fl_line(ix, y1-3, ix, y1+3);
		}
		fx += xscale;
	}
}

//-----------------------------------------------------------------------------
void QPlotWidget::drawAxes(QPainter& p)
{
	// get the center in screen coordinates
	QPoint c = ViewToScreen(QPointF(0.0, 0.0));

	// render the X-axis
	if ((c.y() > m_screenRect.top   ()) &&
		(c.y() < m_screenRect.bottom()))
	{
		QPainterPath xaxis;
		xaxis.moveTo(m_screenRect.left (), c.y());
		xaxis.lineTo(m_screenRect.right(), c.y());
		p.drawPath(xaxis);
	}

	// render the Y-axis
	if ((c.x() > m_screenRect.left ()) &&
		(c.x() < m_screenRect.right()))
	{
		QPainterPath yaxis;
		yaxis.moveTo(c.x(), m_screenRect.top   ());
		yaxis.lineTo(c.x(), m_screenRect.bottom());
		p.drawPath(yaxis);
	}
}

//-----------------------------------------------------------------------------
void QPlotWidget::drawAllData(QPainter& p)
{
	QStringList colorNames = QColor::colorNames();

	int N = m_data.size();
	for (int i=0; i<N; ++i)
	{
		QColor col(colorNames[(i+13)%colorNames.count()]);
		QPen pen(col, 2);
		p.setPen(pen);
		p.setBrush(col);
		drawData(p, m_data[i]);
	}
}

//-----------------------------------------------------------------------------
void QPlotWidget::drawData(QPainter& p, QPlotData& d)
{
	int N = d.size();
	if (N == 0) return;

	QPainterPath path;
	QPoint pt = ViewToScreen(d.Point(0));
	path.moveTo(pt.x(), pt.y());
	QBrush b = p.brush();
	p.setBrush(Qt::NoBrush);
	for (int i=1; i<N; ++i)
	{
		pt = ViewToScreen(d.Point(i));
		path.lineTo(pt.x(), pt.y());
	}
	p.drawPath(path);

	// draw the marks
	p.setBrush(b);
	for (int i=0; i<N; ++i)
	{
		pt = ViewToScreen(d.Point(i));
		QRect r(pt.x()-2, pt.y()-2,5,5);
		p.drawRect(r);
	}
}
