#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <FEBioLib/FEBioModel.h>
#include "PlotWidget.h"
#include "QGLView.h"
#include "DataPlot.h"
#include "ParamInput.h"

class XMLTag;
class FEModel;
class QBoxLayout;

//-----------------------------------------------------------------------------
class ModelData
{
public:
	FEBioModel		m_fem;
	FECoreTask*		m_task;
	std::string 	m_taskFile;

	ModelData::ModelData() : m_task(0) {}

	ModelData::~ModelData() { if (m_task) delete m_task; m_task = 0; }
};

//-----------------------------------------------------------------------------
//! This class represents the GUI
class MyDialog : public QDialog
{
	Q_OBJECT

public:
	MyDialog();

	bool BuildGUI(const char* szfile);

	void AddInputParameter(CParamInput *pi) { m_in.push_back(pi); }

	void AddGraph(CDataPlot* plot) { m_plot.push_back(plot); }

	void AddPlot3D(QGLView* plot3d) { m_gl.push_back(plot3d); }

public slots:
	void ResetDlg();
	void Run();
	void RunTask();

private:
	static bool cb(FEModel* pfem, unsigned int nwhen, void* pd)
	{
		MyDialog* pThis = (MyDialog*) pd;
		return pThis->FECallback(*pfem, nwhen);
	}

	bool FECallback(FEModel& fem, unsigned int nwhen);

private:
	ModelData				m_model;	//!< The model data
	vector<QGLView*>		m_gl;
	vector<CDataPlot*>		m_plot;
	vector<CParamInput*>	m_in;
};
