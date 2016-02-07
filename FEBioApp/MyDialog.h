#pragma once
#include <QDialog>
#include <QLineEdit>
#include <FEBioLib/FEBioModel.h>

class QPlotWidget;
class XMLTag;

class QParamInput : public QLineEdit
{
public:
	QParamInput(QWidget* parent = 0);

	void SetParameter(double* pv);

	void UpdateParameter();

private:
	double*	m_pv;	// pointer to parameter
};

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
	vector<QPlotWidget*>	m_plot;
	vector<QParamInput*>	m_in;
};
