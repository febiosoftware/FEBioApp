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

void CParamInput::SetParameter(FEParam* pv)
{
	m_pv = pv;
	if (pv) setText(QString::number(pv->value<double>()));
}

void CParamInput::UpdateParameter()
{
	QString s = text();
	double f = s.toDouble();
	if (m_pv) 
	{
		printf("Setting parameter %s to %lg\n", m_pv->name(), f);
		m_pv->setvalue(f);
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
		if (tag == "group") parseGroup(tag, playout);
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
		else if (tag == "input") parseInput(tag, playout);
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

void MyDialog::parseGroup(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztype = tag.AttributeValue("type");

	QBoxLayout* pl = 0;
	if      (strcmp(sztype, "vertical"  ) == 0) pl = new QVBoxLayout;
	else if (strcmp(sztype, "horizontal") == 0) pl = new QHBoxLayout;
	playout->addLayout(pl);

	parseTags(tag, pl);
	++tag;
}

void MyDialog::parseInput(XMLTag& tag, QBoxLayout* playout)
{
	char sz[256] = {0};
	strcpy(sz, tag.AttributeValue("title"));

	int nalign = CParamInput::ALIGN_LEFT;
	const char* szalign = tag.AttributeValue("align", true);
	if (szalign)
	{
		if      (strcmp(szalign, "left"       ) == 0) nalign = CParamInput::ALIGN_LEFT;
		else if (strcmp(szalign, "right"      ) == 0) nalign = CParamInput::ALIGN_RIGHT;
		else if (strcmp(szalign, "top"        ) == 0) nalign = CParamInput::ALIGN_TOP;
		else if (strcmp(szalign, "bottom"     ) == 0) nalign = CParamInput::ALIGN_BOTTOM;
		else if (strcmp(szalign, "topleft"    ) == 0) nalign = CParamInput::ALIGN_TOP_LEFT;
		else if (strcmp(szalign, "topright"   ) == 0) nalign = CParamInput::ALIGN_TOP_RIGHT;
		else if (strcmp(szalign, "bottomleft" ) == 0) nalign = CParamInput::ALIGN_BOTTOM_LEFT;
		else if (strcmp(szalign, "bottomright") == 0) nalign = CParamInput::ALIGN_BOTTOM_RIGHT;
		else printf("WARNING: Unknown align value %s\n", szalign);
	}

	XMLReader& xml = *tag.m_preader;

	FEParam* pv = 0;
	if (!tag.isempty())
	{
		++tag;
		do
		{
			if (tag == "param")
			{
				pv = findParameter(tag.szvalue());
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

	QBoxLayout* pl = 0;

	if ((nalign==CParamInput::ALIGN_LEFT)||(nalign==CParamInput::ALIGN_RIGHT)) pl = new QHBoxLayout;
	else pl = new QVBoxLayout;

	QLabel* plabel = new QLabel(sz);
	CParamInput* pi = new CParamInput;
	if (pv) pi->SetParameter(pv);

	playout->addLayout(pl);

	switch (nalign)
	{
	case CParamInput::ALIGN_LEFT:
		pl->addWidget(plabel);
		pl->addStretch();
		pl->addWidget(pi);
		break;
	case CParamInput::ALIGN_RIGHT:
		pl->addWidget(pi);
		pl->addStretch();
		pl->addWidget(plabel);
		break;
	case CParamInput::ALIGN_TOP:
		plabel->setAlignment(Qt::AlignHCenter);
		pl->addWidget(plabel);
		pl->addWidget(pi);
		break;
	case CParamInput::ALIGN_BOTTOM:
		pl->addWidget(pi);
		plabel->setAlignment(Qt::AlignHCenter);
		pl->addWidget(plabel);
		break;
	case CParamInput::ALIGN_TOP_LEFT:
		pl->addWidget(plabel);
		pl->addWidget(pi);
		break;
	case CParamInput::ALIGN_TOP_RIGHT:
		plabel->setAlignment(Qt::AlignRight);
		pl->addWidget(plabel);
		pl->addWidget(pi);
		break;
	case CParamInput::ALIGN_BOTTOM_LEFT:
		pl->addWidget(pi);
		pl->addWidget(plabel);
		break;
	case CParamInput::ALIGN_BOTTOM_RIGHT:
		plabel->setAlignment(Qt::AlignRight);
		pl->addWidget(pi);
		pl->addWidget(plabel);
		break;
	default:
		assert(false);
	}

	m_in.push_back(pi);
					
	++tag;
}

FEParam* MyDialog::findParameter(const char* sz)
{
	ParamString s(sz);
	if (strcmp(s.c_str(), "fem") == 0)
	{
		s = s.next();
		char szbuf[256] = {0}, *szindex = 0;
		int nindex = -1;
		strcpy(szbuf, s.c_str());
		char* ch = strchr(szbuf, '(');
		if (ch)
		{
			*ch++ = 0;
			szindex = ch;
			ch = strchr(ch, ')');
			if (ch == 0) return 0;
			*ch=0;

			if (szindex[0] == '"')
			{
				ch = strchr(++szindex, '"');
				if (ch==0) return 0;
				*ch = 0;
			}
			else
			{
				nindex = atoi(szindex);
				szindex = 0;
			}
		}

		if (strcmp(szbuf, "material") == 0)
		{
			// get the material
			FEMaterial* pmat = 0;
			if (szindex) pmat = m_fem.FindMaterial(szindex);
			else pmat = m_fem.FindMaterial(nindex);
			if (pmat == 0) return 0;

			s = s.next();
			FEParam* pp = pmat->GetParameter(s);
			return pp;
		}
		else if (strcmp(szbuf, "surfaceLoad") == 0)
		{
			// get the surface load
			FESurfaceLoad* psl = 0;
			if (szindex) psl = m_fem.FindSurfaceLoad(szindex);
			if (psl == 0) return 0;

			s = s.next();
			FEParam* pp = psl->GetParameter(s);
			return pp;
		}
	}
	return 0;
}

// Include the MOC stuff
#include "moc_MyDialog.cpp"
