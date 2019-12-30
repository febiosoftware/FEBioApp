#include "stdafx.h"
#include "MyDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include "UIBuilder.h"
#include <QtCore/QCoreApplication>
#include "Interpreter.h"

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

void qt_error(const char* sz)
{
	QMessageBox b;
	b.setIcon(QMessageBox::Critical);
	b.setText(sz);
	b.exec();
}

void qt_info(const char* sz)
{
	QMessageBox b;
	b.setIcon(QMessageBox::Information);
	b.setText(sz);
	b.exec();
}

//-----------------------------------------------------------------------------
CActionButton::CActionButton(QWidget* parent) : QPushButton(parent)
{
	QObject::connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
}

void CActionButton::setCode(const QString& code)
{
	m_code = code;
}

class ButtonVariable : public Code::Variable
{
public:
	ButtonVariable(QPushButton* b) : m_pb(b) {}

	std::string get() override { return m_pb->text().toStdString(); }

	void set(const std::string& s) override { m_pb->setText(QString::fromStdString(s)); }

private:
	QPushButton* m_pb;
};

void CActionButton::onClicked()
{
	Code::Interpreter interpreter;
	ButtonVariable v(this);
	interpreter.addVar("this.text", &v);
	emit runCode(m_code);
}

//-----------------------------------------------------------------------------
MyDialog::MyDialog()
{
	setLayout(new QVBoxLayout);

	QObject::connect(&m_data, SIGNAL(modelInit(int)), this, SLOT(on_modelInit(int)));
	QObject::connect(&m_data, SIGNAL(timeStepDone(int)), this, SLOT(on_timeStepDone(int)));
}

void MyDialog::on_modelInit(int index)
{
	m_startTime = m_lastTime = clock();
	UpdatePlots(true);
}

void MyDialog::on_timeStepDone(int index)
{
	clock_t t1 = clock();

	double sec = (double)(t1 - m_lastTime) / CLOCKS_PER_SEC;

	if (sec > 0.05)
	{
		UpdatePlots(true);

		m_lastTime = t1;

		QCoreApplication::processEvents();
	}
	else UpdatePlots(false);
}

void MyDialog::UpdatePlots(bool breset)
{
	// update the plots
	for (int i = 0; i<(int)m_plot.size(); ++i) m_plot[i]->Update();

	// update all 3D plots
	for (int i = 0; i<(int)m_gl.size(); ++i) m_gl[i]->Update(breset);

}

void MyDialog::doAction(int id, int naction)
{
	if (id == -1)
	{
		if (naction == 0) Quit();
		if (naction == 1) ResetDlg();
	}
	else
	{
		switch (naction)
		{
		case 0: RunModel(id); break;
		case 1: Stop(id); break;
		case 2: Pause(id); break;
		case 3: Continue(id); break;
		}
	}
}

void MyDialog::RunCode(QString& codeBlock)
{
	// clear all plots
	// TODO: remove this. I want the plots detect automatically when they need to reset
	for (int i = 0; i<(int)m_plot.size(); ++i) m_plot[i]->Reset();

	// get the script to run
	std::string code = codeBlock.toStdString();

	// create the interpreter object
	Code::Interpreter interpreter;

	// build the function table
	Code::Interpreter::clearFunctions();

	Code::Interpreter::addFunction("app.quit" , [=]() { this->doAction(-1, 0); });
	Code::Interpreter::addFunction("app.reset", [=]() { this->doAction(-1, 1); });

	int models = m_data.Models();
	for (int i = 0; i < models; ++i)
	{
		std::string fem = m_data.GetModelId(i);

		std::string solve = fem + ".solve";
		std::string stop  = fem + ".stop";
		std::string pause = fem + ".pause";
		std::string conti = fem + ".continue";

		Code::Interpreter::addFunction(solve, [=]() { this->doAction(i, 0); });
		Code::Interpreter::addFunction(stop , [=]() { this->doAction(i, 1); });
		Code::Interpreter::addFunction(pause, [=]() { this->doAction(i, 2); });
		Code::Interpreter::addFunction(conti, [=]() { this->doAction(i, 3); });
	}

	try {
		interpreter.runCode(code);
	}
	catch (...)
	{
		QMessageBox::critical(this, "FEBioApp", "Failed running code");
	}

	// resize all graphs
	for (int i = 0; i<(int)m_plot.size(); ++i) m_plot[i]->UpdatePlots();

	// update all 3D plots
	for (int i = 0; i<(int)m_gl.size(); ++i) m_gl[i]->Update();

	// update GUI
	repaint();
}

void MyDialog::Stop(int modelIndex)
{
	m_data.StopModel(modelIndex);
}

void MyDialog::Quit()
{
	// stop the model if it is running
	m_data.StopAll();

	// close the dialog box
	accept();
}

void MyDialog::closeEvent(QCloseEvent* ev)
{
	// stop the model if it's running
	m_data.StopAll();
}

void MyDialog::RunModel(int modelIndex)
{
	if (m_data.GetFEBioStatus(modelIndex) == FEBioData::RUNNING) return;

	// do initialization
	bool init = true;
	if (m_data.IsModelInitialized(modelIndex) == false)
	{
		init = m_data.InitModel(modelIndex);
	}
	else 
	{
		init = m_data.ResetModel(modelIndex);
	}

	if (init == false)
	{
		QMessageBox::critical(this, "FEBioApp", "Failed to initialize the model. Aborting run.");
		return;
	}

	// solve the model
	QString fileName = QString::fromStdString(m_data.GetModelFile(modelIndex));
	setWindowTitle(QString::fromStdString(m_fileName) + " (Running:" + fileName + ")");

	printf("Calling FEBio ... ");
	if (m_data.SolveModel(modelIndex))
	{
		printf("NORMAL TERMINATION\n");
	}
	else
	{
		if (m_data.ForceStop(modelIndex) == false)
		{
			printf("ERROR TERMINATION\n");
			qt_error("ERROR TERMINATION\n");
		}
		else
		{
			printf("USER TERMINATION\n");
		}
	}
	setWindowTitle(QString::fromStdString(m_fileName));
}

void MyDialog::RunTask()
{
/*	static bool bfirst = true;

	// make sure there is a task
	if (m_data.HasTask() == false)
	{
		printf("No task defined.");
		qt_error("No task defined");
		return;
	}

	// first time we get here, we need to do some initialization
	if (bfirst)
	{	
		bfirst = false;
		
		// initialize the model
		if (m_data.InitModel() == false)
		{
			qt_error("Model failed to initialize");
		}
	}
	else m_data.ResetModel();

	// run the task
	printf("Calling FEBio ... ");
	if (m_data.RunModel())
	{
		printf("NORMAL TERMINATION\n");
	}
	else
	{
		printf("ERROR TERMINATION\n");
		qt_error("ERROR TERMINATION\n");
	}
*/
}

bool MyDialog::BuildGUI(const char* szfile)
{
	const char* fileTitle = 0;
	fileTitle = strrchr(szfile, '/');
	if (fileTitle == 0) 
	{
		fileTitle = strrchr(szfile, '\\');
		if (fileTitle == 0) fileTitle = szfile;
		else fileTitle++;
	}
	else fileTitle++;

	m_fileName = fileTitle;

	setWindowTitle(fileTitle);

	UIBuilder ui;
	if (ui.BuildUI(this, m_data, szfile) == false)
	{
		QMessageBox::critical(this, "", "Failed building UI");
		return false;
	}

	return true;
}

void MyDialog::ResetDlg()
{
	// update input values
	for (int i = 0; i<(int)m_in.size(); ++i)
	{
		m_in[i]->ResetParameter();
	}

	// clear all plots
	for (int i = 0; i<(int)m_plot.size(); ++i) m_plot[i]->clearData();

	// update GUI
	repaint();
}

void MyDialog::Pause(int modelIndex)
{
//	if (m_brunning)
//	{
//		m_bpaused = true;
//	}
}

void MyDialog::Continue(int modelIndex)
{
//	if (m_brunning && m_bpaused)
//	{
//		m_bpaused = false;
//	}
}
