#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QSlider>
#include <FEBioLib/FEBioModel.h>
#include "PlotWidget.h"
#include "QGLView.h"
#include "DataPlot.h"
#include "ParamInput.h"
#include <FECore/FECoreTask.h>

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
	void paramChanged();
	void Stop();
	void Quit();

private:
	static bool cb(FEModel* pfem, unsigned int nwhen, void* pd)
	{
		MyDialog* pThis = (MyDialog*) pd;
		return pThis->FECallback(*pfem, nwhen);
	}

	bool FECallback(FEModel& fem, unsigned int nwhen);

	void UpdateModelParameters();

private:
	ModelData				m_model;	//!< The model data
	vector<QGLView*>		m_gl;
	vector<CDataPlot*>		m_plot;
	vector<CParamInput*>	m_in;

	QString		m_fileName;

	bool	m_bupdateParams;	//!< a parameter has changed
	bool	m_bforceStop;		//!< stop the model to run
	bool	m_brunning;			//!< is model running
};
