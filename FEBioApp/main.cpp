/*This file is part of the FEBioApp source code and is licensed under the MIT license
listed below.

See Copyright-FEBioApp.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "FEBioApp.h"
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QMessageBox>
#include "FEBioAppUIBuilder.h"
#include "FEBioApp.h"
#include "FEBioAppWidget.h"
#include <FEBioLib/febio.h>
#include <FSCore/ColorMapManager.h>

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	QString appPath = QString(argv[1]);

	febio::InitLibrary();

	ColorMapManager::Initialize();

	FEBioApp febioApp;
	FEBioAppUIBuilder uiBuilder;
	FEBioAppWidget* w = uiBuilder.BuildUIFromFile(appPath, &febioApp);
	if (w == nullptr)
	{
		QMessageBox::critical(nullptr, "FEBio Studio", "Failed to read file!");
		return 1;
	}
	else
	{
		febioApp.SetUI(w);
		w->show();
	}

	return app.exec();
}
