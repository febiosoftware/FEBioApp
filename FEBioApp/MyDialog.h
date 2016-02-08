#pragma once
#include <QDialog>
#include <QLineEdit>
#include <FEBioLib/FEBioModel.h>
#include "QPlotWidget.h"
#include "QGLView.h"

class XMLTag;
class FEModel;

//-----------------------------------------------------------------------------
//! This class connects an FE model parameter to an input field.
class CParamInput : public QLineEdit
{
public:
	CParamInput(QWidget* parent = 0);

	void SetParameter(double* pv);

	void UpdateParameter();

private:
	double*	m_pv;	// pointer to parameter
};

//-----------------------------------------------------------------------------
//! Class for plotting data
class CDataPlot : public QPlotWidget
{
public:
	CDataPlot(QWidget* parent = 0) : QPlotWidget(parent)
	{
	}

	void Update(FEModel& fem);
};

//-----------------------------------------------------------------------------
//! This class represents the GUI
class MyDialog : public QDialog
{
	Q_OBJECT

public:
	MyDialog();

	void BuildGUI(const char* szfile);

public Q_SLOTS:
	void Run();	// run the FE model

private:
	static bool cb(FEModel* pfem, unsigned int nwhen, void* pd)
	{
		MyDialog* pThis = (MyDialog*) pd;
		return pThis->FECallback(*pfem, nwhen);
	}

	bool FECallback(FEModel& fem, unsigned int nwhen);

private: // helper functions for parsing app file
	bool parseModel(XMLTag& tag);
	bool parseGUI  (XMLTag& tag);

private:
	char		m_szfile[512];	//!< FE model input file name
	FEBioModel	m_fem;			//!< The FE model
	vector<QGLView*>		m_gl;
	vector<CDataPlot*>		m_plot;
	vector<CParamInput*>	m_in;
};
