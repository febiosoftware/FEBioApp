// FEBioApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <QApplication>
#include "MyDialog.h"
#include <FEBioLib/febio.h>
#include <QSurfaceFormat>

// This is defined in the FEBioLib library
extern int get_app_path (char *pname, size_t pathsize);

int main(int argc, char* argv[])
{
	// initialize the FEBio library
	febio::InitLibrary();

	// read the configuration file
	char szpath[1024] = {0};
	if (get_app_path(szpath, 1023) == 0)
	{
		char sz[1024] = {0};

		char* ch = strrchr(szpath, '\\');
		if (ch == 0) ch = strrchr(szpath, '/');
		if (ch) ch[1] = 0;

		sprintf(sz, "%sfebio.xml", szpath);

		if (febio::Configure(sz) == false)
		{
			printf("ERROR: Failed reading configuration file.\n");
		}
	}

	// create the application
	QApplication app(argc, argv);

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
	dlg.BuildGUI(argv[1]);

	// show the dialog
	dlg.show();

	// run the app
	return app.exec();
}
