#include "stdafx.h"
#include "MyDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include "UIBuilder.h"
#include <QtCore/QCoreApplication>

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
	m_naction[0] = m_naction[1] = -1;
	m_index = 0;
	QObject::connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
}

void CActionButton::onClicked()
{
	int naction = m_naction[m_index];
	
	if (m_index == 0)
	{
		if (m_naction[1] != -1) 
		{
			m_index = 1;
			setText(m_label[m_index]);
		}
	}
	else 
	{
		m_index = 0;
		setText(m_label[m_index]);
	}

	emit doAction(naction);
}

void CActionButton::setAction(int naction, const QString& label, int index)
{
	m_naction[index] = naction;
	m_label[index] = label;
	if (index == 0) setText(label);
}

//-----------------------------------------------------------------------------
MyDialog::MyDialog()
{
	setLayout(new QVBoxLayout);

	m_bupdateParams = true;

	m_bforceStop = false;

	QObject::connect(&m_data, SIGNAL(modelInit()), this, SLOT(on_modelInit()));
	QObject::connect(&m_data, SIGNAL(timeStepDone()), this, SLOT(on_timeStepDone()));
}

void MyDialog::on_modelInit()
{
	m_startTime = m_lastTime = clock();
	UpdatePlots(true);
}

void MyDialog::on_timeStepDone()
{
	clock_t t1 = clock();

	double sec = (double)(t1 - m_lastTime) / CLOCKS_PER_SEC;

	if (sec > 0.05)
	{
		UpdatePlots(false);

		if (m_bupdateParams) UpdateModelParameters();

		m_lastTime = t1;

		QCoreApplication::processEvents();
	}
}

void MyDialog::UpdatePlots(bool breset)
{
	// update the plots
	for (int i = 0; i<(int)m_plot.size(); ++i) m_plot[i]->Update();

	// update all 3D plots
	for (int i = 0; i<(int)m_gl.size(); ++i) m_gl[i]->Update(breset);

}

void MyDialog::UpdateModelParameters()
{
	for (int i = 0; i<(int)m_in.size(); ++i)
	{
		m_in[i]->UpdateParameter();
	}

	m_bupdateParams = false;
}

void MyDialog::doAction(int naction)
{
	switch (naction)
	{
	case 0: Run(); break;
	case 1: Quit(); break;
	case 2: ResetDlg(); break;
	case 3: RunTask(); break;
	case 4: Stop(); break;
	case 5: Pause(); break;
	case 6: Continue(); break;
	}
}

void MyDialog::Stop()
{
	m_bforceStop = true;
	m_data.SetFEBioStatus(FEBioData::STOPPED);
}

void MyDialog::Quit()
{
	// stop the model if it is running
	Stop();

	// close the dialog box
	accept();
}

void MyDialog::closeEvent(QCloseEvent* ev)
{
	// stop the model if it's running
	Stop();
}

void MyDialog::Run()
{
	if (m_data.GetFEBioStatus() == FEBioData::RUNNING) return;

	static bool modelInitialized = false;

	m_bforceStop = false;

	// update input values
	UpdateModelParameters();

	// clear all plots
	for (int i=0; i<(int) m_plot.size(); ++i) m_plot[i]->Reset();

	// do initialization
	if (modelInitialized == false)
	{
		modelInitialized = m_data.InitModel();
	}
	else 
	{
		m_data.ResetModel();
	}

	if (modelInitialized == false)
	{
		QMessageBox::critical(this, "FEBioApp", "Failed to initialize the model. Aborting run.");
		return;
	}

	// solve the model
	setWindowTitle(m_fileName + " (Running)");

	printf("Calling FEBio ... ");
	if (m_data.SolveModel())
	{
		printf("NORMAL TERMINATION\n");
	}
	else
	{
		if (m_bforceStop == false)
		{
			printf("ERROR TERMINATION\n");
			qt_error("ERROR TERMINATION\n");
		}
		else
		{
			printf("USER TERMINATION\n");
		}
	}
	setWindowTitle(m_fileName);

	// resize all graphs
	for (int i=0; i<(int) m_plot.size(); ++i) m_plot[i]->UpdatePlots();

	// update all 3D plots
	for (int i=0; i<(int) m_gl.size(); ++i) m_gl[i]->Update();

	// update GUI
	repaint();
}

void MyDialog::RunTask()
{
	static bool bfirst = true;

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

void MyDialog::paramChanged()
{
	m_bupdateParams = true;
}

void MyDialog::Pause()
{
//	if (m_brunning)
//	{
//		m_bpaused = true;
//	}
}

void MyDialog::Continue()
{
//	if (m_brunning && m_bpaused)
//	{
//		m_bpaused = false;
//	}
}
