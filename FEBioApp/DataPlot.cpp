#include "stdafx.h"
#include "DataPlot.h"

//-----------------------------------------------------------------------------
CDataPlot::CDataPlot(QWidget* parent) : CPlotWidget(parent)
{

}

//-----------------------------------------------------------------------------
void CDataPlot::AddData(CDataSource& data, const QString& label)
{
	m_data.push_back(data);
	CPlotData d;
	d.setLabel(label);
	addPlotData(d);
}

//-----------------------------------------------------------------------------
void CDataPlot::Update(FEModel& fem)
{
	for (int i = 0; i<plots(); ++i)
	{
		CPlotData& plot = getPlotData(i);

		CDataSource& data = m_data[i];

		if (data.m_x.isValid() && data.m_y.isValid())
			plot.addPoint(data.m_x.value<double>(), data.m_y.value<double>());
	}
}
