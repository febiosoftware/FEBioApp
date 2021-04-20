#include "stdafx.h"
#include "DataPlot.h"

//=============================================================================
int CModelDataSource::Points() const
{
	int nx = m_x->Values();
	int ny = m_y->Values();
	int n = (nx < ny ? nx : ny);
	return n;
}
//-----------------------------------------------------------------------------
QPointF CModelDataSource::Point(int i) const
{
	double x = m_x->GetValue(i);
	double y = m_y->GetValue(i);
	return QPointF(x, y);
}

//=============================================================================
CDataPlot::CDataPlot(QWidget* parent) : CPlotWidget(parent)
{

}

//-----------------------------------------------------------------------------
void CDataPlot::AddData(CDataPlotSource* data, const QString& label)
{
	m_data.push_back(data);
	CLineChartData* d = new CLineChartData;
	d->setLabel(label);
	addPlotData(d);
}

//-----------------------------------------------------------------------------
void CDataPlot::UpdatePlots()
{
	clearData();

	for (int i = 0; i<plots(); ++i)
	{
		CPlotData& plot = getPlotData(i);

		CDataPlotSource& data = *m_data[i];

		size_t N = data.Points();
		for (int j=0; j<N; ++j)
		{
			QPointF p = data.Point(j);
			plot.addPoint(p.x(), p.y());
		}
	}

	fitToData();
}
