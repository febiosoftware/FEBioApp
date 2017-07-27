#include "stdafx.h"
#include "MyDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include "UIBuilder.h"

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

	m_fem.AddCallback(cb, CB_ALWAYS, this);
}

bool MyDialog::FECallback(FEModel& fem, unsigned int nwhen)
{
	if ((nwhen == CB_MAJOR_ITERS) || (nwhen == CB_INIT))
	{
		// update all model data
		fem.UpdateModelData();

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

	// do initialization
	if (bfirst)
	{
		m_fem.Init();
		bfirst = false;
	}
	else m_fem.Reset();

	// solve the model
	printf("Calling FEBio ... ");
	if (m_fem.Solve())
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

bool MyDialog::BuildGUI(const char* szfile)
{
	setWindowTitle(szfile);

	UIBuilder ui;
	if (ui.BuildUI(this, m_fem, szfile) == false)
	{
		QMessageBox::critical(this, "", "Failed building UI");
		return false;
	}

	return true;
}
