#pragma once
#include "PlotWidget.h"
#include <FECore/FEParam.h>
#include <FECore/FEModel.h>

class CDataSource
{
public:
	CDataSource() {}
	virtual ~CDataSource() {}

	virtual void Update() {}

	virtual void Reset() {}

public:
	vector<QPointF>	m_data;
};

//-----------------------------------------------------------------------------
class CParamDataSource : public CDataSource
{
public:
	FEParamValue	m_x;
	FEParamValue	m_y;

	void Reset() { m_data.clear(); }

	void Update()
	{
		QPointF p;
		p.setX(m_x.value<double>());
		p.setY(m_y.value<double>());
		m_data.push_back(p);
	}
};

//-----------------------------------------------------------------------------
class CStaticDataSource : public CDataSource
{
};

//-----------------------------------------------------------------------------
//! Class for plotting data
class CDataPlot : public CPlotWidget
{
public:
	CDataPlot(QWidget* parent = 0);

	void Update(FEModel& fem);

	void Reset();

	void AddData(CDataSource* data, const QString& label);

	void UpdatePlots();

private:
	vector<CDataSource*>	m_data;
};
