// FEBioApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <QApplication>
#include "MyDialog.h"
#include <FEBioLib/febio.h>

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

	QApplication app(argc, argv);

	MyDialog dlg;
	dlg.setAttribute(Qt::WA_QuitOnClose);

	dlg.BuildGUI(argv[1]);

	dlg.show();

	return app.exec();
}
