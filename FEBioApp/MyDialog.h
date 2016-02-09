#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <FEBioLib/FEBioModel.h>
#include "QPlotWidget.h"
#include "QGLView.h"

class XMLTag;
class FEModel;
class QBoxLayout;

//-----------------------------------------------------------------------------
//! This class connects an FE model parameter to an input field.
class CParamInput
{
public:
	enum {
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_TOP,
		ALIGN_BOTTOM,
		ALIGN_TOP_LEFT,
		ALIGN_TOP_RIGHT,
		ALIGN_BOTTOM_LEFT,
		ALIGN_BOTTOM_RIGHT
	};

public:
	CParamInput();

	void SetWidget(QLineEdit* pw) { m_pedit = pw; }
	void SetWidget(QCheckBox* pw) { m_pcheck = pw; }

	void SetParameter(FEParam* pv);

	void UpdateParameter();

private:
	QLineEdit*	m_pedit;
	QCheckBox*	m_pcheck;

	FEParam*	m_pv;	// pointer to parameter
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
	bool parseTags   (XMLTag& tag, QBoxLayout* playout);
	void parseGroup  (XMLTag& tag, QBoxLayout* playout);
	void parseInput  (XMLTag& tag, QBoxLayout* playout);
	void parseStretch(XMLTag& tag, QBoxLayout* playout);
	void parseButton (XMLTag& tag, QBoxLayout* playout);
	void parseLabel  (XMLTag& tag, QBoxLayout* playout);
	void parseGraph  (XMLTag& tag, QBoxLayout* playout);
	void parsePlot3d (XMLTag& tag, QBoxLayout* playout);

	FEParam* findParameter(const char* sz);

private:
	char		m_szfile[512];	//!< FE model input file name
	FEBioModel	m_fem;			//!< The FE model
	vector<QGLView*>		m_gl;
	vector<CDataPlot*>		m_plot;
	vector<CParamInput*>	m_in;
};
