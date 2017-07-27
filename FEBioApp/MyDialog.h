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

public Q_SLOTS:
	void Run();	// run the FE model

private:
	static bool cb(FEModel* pfem, unsigned int nwhen, void* pd)
	{
		MyDialog* pThis = (MyDialog*) pd;
		return pThis->FECallback(*pfem, nwhen);
	}

	bool FECallback(FEModel& fem, unsigned int nwhen);

private:
	FEBioModel	m_fem;			//!< The FE model
	vector<QGLView*>		m_gl;
	vector<CDataPlot*>		m_plot;
	vector<CParamInput*>	m_in;
};
