#pragma once
#include "PlotWidget.h"
#include <FECore/FEParam.h>
#include <FECore/FEModel.h>

//-----------------------------------------------------------------------------
class CDataSource
{
public:
	FEParamValue	m_x;
	FEParamValue	m_y;
};

//-----------------------------------------------------------------------------
//! Class for plotting data
class CDataPlot : public CPlotWidget
{
public:
	CDataPlot(QWidget* parent = 0);

	void Update(FEModel& fem);

	void AddData(CDataSource& data, const QString& label);

private:
	vector<CDataSource>	m_data;
};
