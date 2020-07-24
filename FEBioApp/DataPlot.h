#pragma once
#include "PlotWidget.h"
#include <FEBioAppLib/FEModelValuator.h>

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
	FEModelValuator*	m_x;
	FEModelValuator*	m_y;

	void Reset()
	{ 
		m_data.clear(); 
	}

	void Update()
	{
		QPointF p(0., 0.);
		if (m_x) p.setX(m_x->GetValue());
		if (m_y) p.setY(m_y->GetValue());
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

	void Update();

	void Reset();

	void AddData(CDataSource* data, const QString& label);

	void UpdatePlots(bool bclear = true);

private:
	vector<CDataSource*>	m_data;
};
