#include "stdafx.h"
#include "DataPlot.h"

//-----------------------------------------------------------------------------
CDataPlot::CDataPlot(QWidget* parent) : CPlotWidget(parent)
{

}

//-----------------------------------------------------------------------------
void CDataPlot::AddData(CDataSource* data, const QString& label)
{
	m_data.push_back(data);
	CLineChartData* d = new CLineChartData;
	d->setLabel(label);
	addPlotData(d);
}

//-----------------------------------------------------------------------------
void CDataPlot::Reset()
{
	for (int i = 0; i<plots(); ++i)
	{
		CDataSource& data = *m_data[i];
		data.Reset();
	}
}

//-----------------------------------------------------------------------------
void CDataPlot::Update(FEModel& fem)
{
	for (int i = 0; i<plots(); ++i)
	{
		CDataSource& data = *m_data[i];
		data.Update();
	}
}

//-----------------------------------------------------------------------------
void CDataPlot::UpdatePlots()
{
	clearData();

	for (int i = 0; i<plots(); ++i)
	{
		CPlotData& plot = getPlotData(i);

		CDataSource& data = *m_data[i];

		int N = data.m_data.size();
		for (int j=0; j<N; ++j)
		{
			plot.addPoint(data.m_data[j].x(), data.m_data[j].y());
		}
	}

	fitToData();
}
