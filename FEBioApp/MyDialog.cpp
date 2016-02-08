#include "stdafx.h"
#include "MyDialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <FEBioXML/XMLReader.h>
#include <FEBioMech/FEElasticMaterial.h>
#include "QPlotWidget.h"
#include "QGLView.h"

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
CParamInput::CParamInput(QWidget* parent) : QLineEdit(parent)
{
	m_pv = 0;
}

void CParamInput::SetParameter(double* pv)
{
	m_pv = pv;
	if (pv) setText(QString::number(*pv));
}

void CParamInput::UpdateParameter()
{
	QString s = text();
	double f = s.toDouble();
	if (m_pv) 
	{
		printf("Setting paramter to %lg\n", f);
		*m_pv = f;
	}
}

//-----------------------------------------------------------------------------
void CDataPlot::Update(FEModel& fem)
{
	double t = fem.m_ftime;
	FEMesh& mesh = fem.GetMesh();
	FEElement& el = mesh.Domain(0).ElementRef(0);

	FEMaterialPoint& mp = *el.GetMaterialPoint(0);
	FEElasticMaterialPoint& ep = *mp.ExtractData<FEElasticMaterialPoint>();

	mat3ds C = ep.RightCauchyGreen();
	mat3ds s = ep.m_s;

	mat3dd I(1.0);
	mat3ds E = (C - I)*0.5;

	m_data[0].addPoint(E.xx(), s.xx());
}

//-----------------------------------------------------------------------------
MyDialog::MyDialog()
{
	QVBoxLayout* l = new QVBoxLayout(this);

	m_fem.AddCallback(cb, CB_ALWAYS, this);
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
	for (int i=0; i<(int) m_plot.size(); ++i) m_plot[i]->clear();

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

void MyDialog::BuildGUI(const char* szfile)
{
	XMLReader xml;
	if (xml.Open(szfile) == false) return;

	setWindowTitle(szfile);

	XMLTag tag;
	if (xml.FindTag("febio_app", tag) == false) return;

	++tag;
	do
	{
		if (tag == "Model") parseModel(tag);
		if (tag == "GUI"  ) parseGUI  (tag);
		else xml.SkipTag(tag);
	}
	while (!tag.isend());

	xml.Close();
}

bool MyDialog::parseModel(XMLTag& tag)
{
	XMLReader& xml = *tag.m_preader;
	++tag;
	do
	{
		if (tag == "file")
		{
			strcpy(m_szfile, tag.szvalue());

			// Try to load the FE model file
			if (m_fem.Input(m_szfile))
			{
				printf("Success loading input file %s\n", m_szfile);
			}
			else
			{
				printf("Failed loading input file %s\n", m_szfile);
			}

			++tag;
		}
		else xml.SkipTag(tag);
	}
	while (!tag.isend());

	return true;
}

bool MyDialog::parseGUI(XMLTag& tag)
{
	QBoxLayout* playout = dynamic_cast<QBoxLayout*>(layout());

	// read the window title
	char sz[256] = {0};
	strcpy(sz, tag.AttributeValue("title"));

	if (strcmp(sz, "$(filename)") == 0) strcmp(sz, m_szfile);
	else setWindowTitle(sz);


	return parseTags(tag, playout);
}

bool MyDialog::parseTags(XMLTag& tag, QBoxLayout* playout)
{
	// get the file reader
	XMLReader& xml = *tag.m_preader;

	// buffer for reading strings
	char sz[256] = {0};

	// loop over all tags
	++tag;
	do
	{
		if (tag == "group")
		{
			const char* sztype = tag.AttributeValue("type");

			QBoxLayout* pl = 0;
			if      (strcmp(sztype, "vertical"  ) == 0) pl = new QVBoxLayout;
			else if (strcmp(sztype, "horizontal") == 0) pl = new QHBoxLayout;
			playout->addLayout(pl);

			parseTags(tag, pl);
			++tag;
		}
		else if (tag == "stretch")
		{
			playout->addStretch();
			++tag;
		}
		else if (tag == "button")
		{
			int nact = -1;
			strcpy(sz, tag.AttributeValue("title"));

			if (!tag.isempty())
			{
				++tag;
				do
				{
					if (tag == "action")
					{
						const char* sza = tag.szvalue();
						if      (strcmp(sza, "fem.solve()") == 0) nact = 0;
						else if (strcmp(sza, "app.quit()" ) == 0) nact = 1;
						else printf("ERROR: Do not understand action\n");

						++tag;
					}
					else xml.SkipTag(tag);
				}
				while (!tag.isend());
			}

			QHBoxLayout* pl = new QHBoxLayout;
			QPushButton* pb = new QPushButton(sz);

			playout->addLayout(pl);
			pl->addStretch();
			pl->addWidget(pb);

			if      (nact == 0) connect(pb, SIGNAL(clicked()), this, SLOT(Run()));
			else if (nact == 1) connect(pb, SIGNAL(clicked()), this, SLOT(accept()));

			++tag;
		}
		else if (tag == "label")
		{
			strcpy(sz, tag.AttributeValue("title"));

			if (!tag.isempty())
			{
				++tag;
				do
				{
					xml.SkipTag(tag);
				}
				while (!tag.isend());
			}

			QLabel* plabel = new QLabel(sz);
			QFont f("Times", 14, QFont::Bold);
			plabel->setFont(f);
			playout->addWidget(plabel);

			++tag;
		}
		else if (tag == "input")
		{
			strcpy(sz, tag.AttributeValue("title"));

			double* pv = 0;
			if (!tag.isempty())
			{
				++tag;
				do
				{
					if (tag == "param")
					{
						pv = m_fem.FindParameter(tag.szvalue());
						if (pv == 0) 
						{
							printf("ERROR: Failed finding parameter %s\n", tag.szvalue());
						}
						++tag;
					}
					else xml.SkipTag(tag);
				}
				while (!tag.isend());
			}

			QHBoxLayout* pl = new QHBoxLayout;

			QLabel* plabel = new QLabel(sz);
			CParamInput* pi = new CParamInput;
			if (pv) pi->SetParameter(pv);

			playout->addLayout(pl);
			pl->addWidget(plabel);
			pl->addStretch();
			pl->addWidget(pi);

			m_in.push_back(pi);
					
			++tag;
		}
		else if (tag == "graph")
		{
			strcpy(sz, tag.AttributeValue("title"));

			if (!tag.isleaf())
			{
				++tag;
				do
				{
					xml.SkipTag(tag);
				}
				while (!tag.isend());
			}

			CDataPlot* pg = new CDataPlot;
			pg->setTitle(QString(sz));
			playout->addWidget(pg);
			m_plot.push_back(pg);

			++tag;
		}
		else if (tag == "plot3d")
		{
			strcpy(sz, tag.AttributeValue("title"));

			if (!tag.isleaf())
			{
				++tag;
				do
				{
					xml.SkipTag(tag);
				}
				while (!tag.isend());
			}

			QGLView* pgl = new QGLView;
			playout->addWidget(pgl);

			pgl->SetFEModel(&m_fem);
			m_gl.push_back(pgl);

			++tag;
		}
		else xml.SkipTag(tag);
	}
	while (!tag.isend());

	return true;
}

// Include the MOC stuff
#include "moc_MyDialog.cpp"
