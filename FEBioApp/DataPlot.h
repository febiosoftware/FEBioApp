#pragma once
#include "PlotWidget.h"
#include <FEBioAppLib/FEModelValuator.h>

class CDataPlotSource
{
public:
	CDataPlotSource() {}
	virtual ~CDataPlotSource() {}

public:
	virtual int Points() const = 0;

	virtual QPointF Point(int i) const = 0;

protected:
	vector<QPointF>	m_data;
};

//-----------------------------------------------------------------------------
class CModelDataSource : public CDataPlotSource
{
public:
	FEModelValuator*	m_x;
	FEModelValuator*	m_y;

	int Points() const override;
	QPointF Point(int i) const;
};

//-----------------------------------------------------------------------------
class CStaticDataSource : public CDataPlotSource
{
public:
	CStaticDataSource() {}

	void AddPoint(const QPointF& p) { m_data.push_back(p); }

	int Points() const override { return (int)m_data.size(); }
	QPointF Point(int i) const { return m_data[i]; }

private:
	vector<QPointF>	m_data;
};

//-----------------------------------------------------------------------------
//! Class for plotting data
class CDataPlot : public CPlotWidget
{
public:
	CDataPlot(QWidget* parent = 0);

	void AddData(CDataPlotSource* data, const QString& label);

	void UpdatePlots();

private:
	vector<CDataPlotSource*>	m_data;
};
