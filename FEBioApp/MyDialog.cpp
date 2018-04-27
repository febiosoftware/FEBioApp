#include "stdafx.h"
#include "MyDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include "UIBuilder.h"
#include <FECore/FECoreTask.h>
#include <QtCore/QCoreApplication>
#include <time.h>

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
MyDialog::MyDialog()
{
	setLayout(new QVBoxLayout);

	m_bupdateParams = true;

	m_bforceStop = false;
	m_brunning = false;

	m_model.m_fem.AddCallback(cb, CB_ALWAYS, this);
}

bool MyDialog::FECallback(FEModel& fem, unsigned int nwhen)
{
	if (m_bforceStop) return false;

	static clock_t t0, t1;
	if (nwhen == CB_INIT)
	{
		t0 = clock();
	}

	if ((nwhen == CB_MAJOR_ITERS) || (nwhen == CB_INIT))
	{
		t1 = clock();

		double sec = (double)(t1 - t0) / CLOCKS_PER_SEC;

		if (sec > 0.05)
		{
			// update the plots
			for (int i=0; i<(int) m_plot.size(); ++i) m_plot[i]->Update(fem);

			// update all 3D plots
			for (int i = 0; i<(int)m_gl.size(); ++i) m_gl[i]->Update(nwhen == CB_INIT);

			t0 = t1;
		}
	}

	if ((nwhen == CB_MAJOR_ITERS) && (m_bupdateParams))
	{
		UpdateModelParameters();
	}

	QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

	return true;
}

void MyDialog::UpdateModelParameters()
{
	for (int i = 0; i<(int)m_in.size(); ++i)
	{
		m_in[i]->UpdateParameter();
	}

	m_bupdateParams = false;
}

void MyDialog::Stop()
{
	m_bforceStop = true;
}

void MyDialog::Quit()
{
	// stop the model if it is running
	Stop();

	// cloe the dialog box
	accept();
}

void MyDialog::Run()
{
	if (m_brunning) return;

	static bool bfirst = true;

	// make sure stop flag is off
	m_bforceStop = false;

	// update input values
	UpdateModelParameters();

	// clear all plots
	for (int i=0; i<(int) m_plot.size(); ++i) m_plot[i]->Reset();

	FEBioModel& fem = m_model.m_fem;

	// do initialization
	if (bfirst)
	{
		fem.Init();
		bfirst = false;
	}
	else 
	{
		fem.Reset();
	}

	// solve the model
	setWindowTitle(m_fileName + " (Running)");

	m_brunning = true;
	printf("Calling FEBio ... ");
	if (fem.Solve())
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
	m_brunning = false;

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
	if (m_model.m_task == 0)
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
		if (m_model.m_fem.Init() == false)
		{
			qt_error("Model failed to initialize");
		}

		if (m_model.m_task->Init(m_model.m_taskFile.c_str()) == false)
		{
			qt_error("Failed initializing task");
		}
	}
	else m_model.m_fem.Reset();

	// run the task
	printf("Calling FEBio ... ");
	if (m_model.m_task->Run())
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
	if (ui.BuildUI(this, m_model, szfile) == false)
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
