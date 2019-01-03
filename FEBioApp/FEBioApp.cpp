// FEBioApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <QApplication>
#include "MyDialog.h"
#include <FEBioLib/febio.h>
#include <FEBioLib/version.h>
#include <QSurfaceFormat>
#include <iostream>

bool configure();

int main(int argc, char* argv[])
{
	// initialize the FEBio library
	febio::InitLibrary();

#ifdef _DEBUG
	printf("FEBio version %d.%d.%d\n\n", VERSION, SUBVERSION, SUBSUBVERSION);
#endif

	// read the configuration file
	if (configure() == false)
	{
		printf("ERROR: Failed reading configuration file.\n");
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

bool configure()
{
	char szpath[1024] = { 0 };
	if (febio::get_app_path(szpath, 1023) == 0)
	{
		char sz[1024] = { 0 };

		char* ch = strrchr(szpath, '\\');
		if (ch == 0) ch = strrchr(szpath, '/');
		if (ch) ch[1] = 0;

		sprintf(sz, "%sfebio.xml", szpath);

		return febio::Configure(sz);
	}

	return false;
}
