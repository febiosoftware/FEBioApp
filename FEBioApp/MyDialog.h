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

	void setAction(int naction, const QString& label, int index = 0);

signals:
	void doAction(int naction);

protected slots:
	void onClicked();

private:
	int			m_index;
	int			m_naction[2];
	QString		m_label[2];
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
	void Run();
	void RunTask();
	void paramChanged();
	void Stop();
	void Quit();
	void Pause();
	void Continue();

	void doAction(int naction);

	void on_modelInit();
	void on_timeStepDone();

private:
	void UpdateModelParameters();
	void UpdatePlots(bool breset);

private:
	FEBioData				m_data;	//!< The model data
	vector<QGLView*>		m_gl;
	vector<CDataPlot*>		m_plot;
	vector<CParamInput*>	m_in;

	QString		m_fileName;

	bool	m_bupdateParams;	//!< a parameter has changed

	bool	m_bforceStop;

	clock_t	m_startTime, m_lastTime;
};
