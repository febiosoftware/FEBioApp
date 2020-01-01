#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QSlider>
#include <QPushButton>
#include "PlotWidget.h"
#include "QGLView.h"
#include "DataPlot.h"
#include "ParamInput.h"
#include <FEBioAppLib\FEBioData.h>
#include <time.h>

class QBoxLayout;

//-----------------------------------------------------------------------------
class CActionButton : public QPushButton
{
	Q_OBJECT

public:
	CActionButton(QWidget* parent = 0);

	void setCode(const QString& code);

signals:
	void runCode(QString& code);

protected slots:
	void onClicked();

private:
	QString		m_code;
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

	void closeEvent(QCloseEvent* ev) override;

public slots:
	void ResetDlg();
	void RunModel(int modelIndex);
	void Stop(int modelIndex);
	void Quit();
	void Pause(int modelIndex);
	void Continue(int modelIndex);

	void doAction(int id, int naction);
	void RunCode(QString& code);

	void on_modelInit(int index);
	void on_timeStepDone(int index);
	void on_modelReset(int index);

private:
	void UpdatePlots(bool breset);

private:
	FEBioData				m_data;	//!< The model data
	vector<QGLView*>		m_gl;
	vector<CDataPlot*>		m_plot;
	vector<CParamInput*>	m_in;

	std::string		m_fileName;

	clock_t	m_startTime, m_lastTime;
};
