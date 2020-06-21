#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QSlider>
#include <QPushButton>
#include <time.h>
#include <vector>

class QBoxLayout;
class CParamInput;
class CDataPlot;
class QGLView;
class FEBioData;

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
	~MyDialog();

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
	FEBioData*					m_data;	//!< The model data
	std::vector<QGLView*>		m_gl;
	std::vector<CDataPlot*>		m_plot;
	std::vector<CParamInput*>	m_in;

	std::string		m_fileName;

	clock_t	m_startTime, m_lastTime;
};
