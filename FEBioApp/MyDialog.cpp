#include "stdafx.h"
#include "MyDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include "UIBuilder.h"
#include <FECore/FECoreTask.h>

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

	m_model.m_fem.AddCallback(cb, CB_ALWAYS, this);
}

bool MyDialog::FECallback(FEModel& fem, unsigned int nwhen)
{
	if ((nwhen == CB_MAJOR_ITERS) || (nwhen == CB_INIT))
	{
		// update the plots
		for (int i=0; i<(int) m_plot.size(); ++i) m_plot[i]->Update(fem);
	}
	return true;
}

void MyDialog::Run()
{
	static bool bfirst = true;

	// update input values
	for (int i=0; i<(int) m_in.size(); ++i)
	{
		m_in[i]->UpdateParameter();
	}

	// clear all plots
	for (int i=0; i<(int) m_plot.size(); ++i) m_plot[i]->clearData();

	FEBioModel& fem = m_model.m_fem;

	// do initialization
	if (bfirst)
	{
		fem.Init();
		bfirst = false;
	}
	else fem.Reset();

	// solve the model
	printf("Calling FEBio ... ");
	if (fem.Solve())
	{
		printf("NORMAL TERMINATION\n");
	}
	else
	{
		printf("ERROR TERMINATION\n");
		qt_error("ERROR TERMINATION\n");
	}

	// resize all graphs
	for (int i=0; i<(int) m_plot.size(); ++i) m_plot[i]->fitToData();

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
	setWindowTitle(szfile);

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
