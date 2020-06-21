// FEBioApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <QApplication>
#include "MyDialog.h"
#include <QSurfaceFormat>
#include <iostream>
#include "PlotWidget.h"
#include "QGLView.h"
#include "DataPlot.h"
#include "ParamInput.h"
#include <FEBioAppLib\FEBioData.h>

int main(int argc, char* argv[])
{
	// initialize FEBio
	if (FEBioData::InitFEBio() == false)
	{
		printf("ERROR: Failed to initialize FEBio.\n");
	}

	// create the application
	QApplication app(argc, argv);
	app.setApplicationDisplayName("FEBioApp");

	// set the OpenGL default format
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
//	QSurfaceFormat::setDefaultFormat(format);

	// create the dialog
	MyDialog dlg;
	dlg.setAttribute(Qt::WA_QuitOnClose);

	// Build the GUI from the app's file
	if (argc == 2)
	{
		if (dlg.BuildGUI(argv[1]) == false) return 1;

		// show the dialog
		dlg.show();

		// run the app
		return app.exec();
	}
	else
	{
		std::cout << "\nUsage: febioapp.exe inputfile\n\n";
		return 0;
	}
}
