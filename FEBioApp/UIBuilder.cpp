#include "stdafx.h"
#include "UIBuilder.h"
#include <QDialog>
#include <FEBioXML/XMLReader.h>
#include <FEBioMech/FEElasticMaterial.h>
#include <FECore/FESurfaceLoad.h>
#include <FECore/FEParam.h>
#include <FECore/ParamString.h>
#include <FECore/FEModel.h>
#include <FEBioLib/FEBioModel.h>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include "PlotWidget.h"
#include "QGLView.h"
#include "ParamInput.h"
#include "DataPlot.h"
#include "MyDialog.h"

UIBuilder::UIBuilder()
{
	m_fem = 0;
	m_dlg = 0;
}

bool UIBuilder::BuildUI(MyDialog* dlg, FEBioModel& fem, const char* szfile)
{
	if (dlg == 0) return false;

	XMLReader xml;
	if (xml.Open(szfile) == false) return false;

	m_dlg = dlg;
	m_fem = &fem;

	strcpy(m_szfile, szfile);

	XMLTag tag;
	if (xml.FindTag("febio_app", tag) == false) return false;

	++tag;
	do
	{
		if      (tag == "Model") { if (parseModel(tag) == false) return false; }
		else if (tag == "GUI"  ) { if (parseGUI  (tag) == false) return false; }
		else xml.SkipTag(tag);

		tag++;
	}
	while (!tag.isend());

	xml.Close();

	return true;
}

bool UIBuilder::parseModel(XMLTag& tag)
{
	XMLReader& xml = *tag.m_preader;
	++tag;
	do
	{
		if (tag == "file")
		{
			strcpy(m_szfile, tag.szvalue());

			// Try to load the FE model file
			if (m_fem->Input(m_szfile))
			{
				printf("Success loading input file %s\n", m_szfile);
			}
			else
			{
				printf("Failed loading input file %s\n", m_szfile);
				return false;
			}

			++tag;
		}
		else xml.SkipTag(tag);
	}
	while (!tag.isend());

	return true;
}

bool UIBuilder::parseGUI(XMLTag& tag)
{
	QBoxLayout* playout = dynamic_cast<QBoxLayout*>(m_dlg->layout());

	// read the window title
	char sz[256] = {0};
	strcpy(sz, tag.AttributeValue("title"));

	if (strcmp(sz, "$(filename)") == 0) strcmp(sz, m_szfile);
	else m_dlg->setWindowTitle(sz);


	return parseTags(tag, playout);
}

bool UIBuilder::parseTags(XMLTag& tag, QBoxLayout* playout)
{
	// get the file reader
	XMLReader& xml = *tag.m_preader;

	// loop over all tags
	++tag;
	do
	{
		if      (tag == "group"     ) parseGroup    (tag, playout);
		else if (tag == "vgroup"    ) parseVGroup   (tag, playout);
		else if (tag == "hgroup"    ) parseHGroup   (tag, playout);
		else if (tag == "tab_group" ) parseTabGroup (tag, playout);
		else if (tag == "stretch"   ) parseStretch  (tag, playout);
		else if (tag == "button"    ) parseButton   (tag, playout);
		else if (tag == "label"     ) parseLabel    (tag, playout);
		else if (tag == "input"     ) parseInput    (tag, playout);
		else if (tag == "input_list") parseInputList(tag, playout);
		else if (tag == "graph"     ) parseGraph    (tag, playout);
		else if (tag == "plot3d"    ) parsePlot3d   (tag, playout);
		else xml.SkipTag(tag);
	}
	while (!tag.isend());

	return true;
}

void UIBuilder::parseStretch(XMLTag& tag, QBoxLayout* playout)
{
	playout->addStretch();
	++tag;
}

void UIBuilder::parseGroup(XMLTag& tag, QBoxLayout* playout)
{
	const char* szalign = tag.AttributeValue("align");
	const char* szname = tag.AttributeValue("title", true);

	QGroupBox* pg = 0;
	if (szname) pg = new QGroupBox(szname);

	QBoxLayout* pl = 0;
	if      (strcmp(szalign, "vertical"  ) == 0) pl = new QVBoxLayout;
	else if (strcmp(szalign, "horizontal") == 0) pl = new QHBoxLayout;

	parseTags(tag, pl);

	if (pg) 
	{ 
		pg->setLayout(pl); 
		playout->addWidget(pg); 
	}
	else playout->addLayout(pl);

	++tag;
}

void UIBuilder::parseVGroup(XMLTag& tag, QBoxLayout* playout)
{
	const char* szname = tag.AttributeValue("title", true);

	QGroupBox* pg = 0;
	if (szname) pg = new QGroupBox(szname);

	QBoxLayout* pl = new QVBoxLayout;
	parseTags(tag, pl);

	if (pg) 
	{ 
		pl->addStretch();
		pg->setLayout(pl); 
		playout->addWidget(pg); 
	}
	else playout->addLayout(pl);

	++tag;
}

void UIBuilder::parseHGroup(XMLTag& tag, QBoxLayout* playout)
{
	const char* szname = tag.AttributeValue("title", true);

	QGroupBox* pg = 0;
	if (szname) pg = new QGroupBox(szname);

	QBoxLayout* pl = new QHBoxLayout;
	parseTags(tag, pl);

	if (pg) 
	{ 
		pg->setLayout(pl); 
		playout->addWidget(pg); 
	}
	else playout->addLayout(pl);

	++tag;
}

void UIBuilder::parseTabGroup(XMLTag& tag, QBoxLayout* playout)
{
	QTabWidget* ptab = new QTabWidget();

	XMLReader& xml = *tag.m_preader;

	++tag;
	do
	{
		if (tag=="tab")
		{
			QString s(tag.AttributeValue("title"));

			QWidget* pw = new QWidget;
			QVBoxLayout* pl = new QVBoxLayout;
			parseTags(tag, pl);
			pw->setLayout(pl);
			ptab->addTab(pw, s);

			++tag;
		}
		else xml.SkipTag(tag);
	}
	while (!tag.isend());

	playout->addWidget(ptab);

	++tag;
}

void UIBuilder::parseButton(XMLTag& tag, QBoxLayout* playout)
{
	int nact = -1;
	char sz[256] = {0};
	strcpy(sz, tag.AttributeValue("title"));

	XMLReader& xml = *tag.m_preader;

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
				else if (strcmp(sza, "app.reset()") == 0) nact = 2;
				else printf("ERROR: Do not understand action %s\n", sza);

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

	if      (nact == 0) QObject::connect(pb, SIGNAL(clicked()), m_dlg, SLOT(Run()));
	else if (nact == 1) QObject::connect(pb, SIGNAL(clicked()), m_dlg, SLOT(accept()));
	else if (nact == 2) QObject::connect(pb, SIGNAL(clicked()), m_dlg, SLOT(ResetDlg()));

	++tag;
}

void UIBuilder::parseLabel(XMLTag& tag, QBoxLayout* playout)
{
	char sz[256] = {0};
	strcpy(sz, tag.AttributeValue("title"));

	XMLReader& xml = *tag.m_preader;

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
	plabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	playout->addWidget(plabel);

	++tag;
}

void UIBuilder::parseGraph(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztitle = tag.AttributeValue("title");

	// size of graph
	int size[2] = {400, 400};

	XMLReader& xml = *tag.m_preader;

	int nplt = 0;

	FEParamValue xparam, yparam;

	CDataPlot* pg = new CDataPlot(0);
	pg->setTitle(QString(sztitle));

	if (!tag.isleaf())
	{
		++tag;
		do
		{
			if (tag == "size")
			{
				int user_size[2];
				int nread = tag.value(user_size, 2);
				if (nread == 2)
				{
					if (user_size[0] > 100) size[0] = user_size[0];
					if (user_size[1] > 100) size[1] = user_size[1];
				}
				++tag;
			}
			else if (tag == "data")
			{
				nplt++;

				char szname[256] = {0};
				const char* sz = tag.AttributeValue("title", true);
				if (sz) strcpy(szname, sz);
				else sprintf(szname, "plot%d", nplt);

				++tag;
				do
				{
					if (tag == "x")
					{
						ParamString x_str(tag.szvalue());
						xparam = m_fem->FindParameter(x_str);
					}
					else if (tag == "y")
					{
						ParamString y_str(tag.szvalue());
						yparam = m_fem->FindParameter(y_str);
					}
					++tag;
				}
				while (!tag.isend());

				CDataSource src;
				src.m_x = xparam;
				src.m_y = yparam;

				pg->AddData(src, szname);

				xml.SkipTag(tag);
			}
			else xml.SkipTag(tag);
		}
		while (!tag.isend());
	}

	pg->setMinimumSize(QSize(size[0], size[1]));

	playout->addWidget(pg, 1);
	m_dlg->AddGraph(pg);

	++tag;
}

void UIBuilder::parsePlot3d(XMLTag& tag, QBoxLayout* playout)
{
	char sz[256] = {0};
	strcpy(sz, tag.AttributeValue("title"));

	// size of plot view
	int size[2] = {400, 400};

	XMLReader& xml = *tag.m_preader;
	if (!tag.isleaf())
	{
		++tag;
		do
		{
			if (tag == "size")
			{
				int user_size[2];
				int nread = tag.value(user_size, 2);
				if (nread == 2)
				{
					if (user_size[0] > 100) size[0] = user_size[0];
					if (user_size[1] > 100) size[1] = user_size[1];
				}
				++tag;
			}
			else xml.SkipTag(tag);
		}
		while (!tag.isend());
	}

	QGLView* pgl = new QGLView(0, size[0], size[1]);
	playout->addWidget(pgl);

	pgl->SetFEModel(m_fem);
	m_dlg->AddPlot3D(pgl);

	++tag;
}

void UIBuilder::parseInputList(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztitle = tag.AttributeValue("title", true);

	ParamString ps(tag.szvalue());
	FECoreBase* pc = m_fem->FindComponent(ps);
	if (pc)
	{
		QGroupBox* pg = 0;
		if (sztitle) pg = new QGroupBox(sztitle);

		QFormLayout* pf = new QFormLayout;
		FEParameterList& pl = pc->GetParameterList();
		int n = pl.Parameters();
		list<FEParam>::iterator it = pl.first();
		for (int i=0; i<n; ++i, ++it)
		{
			FEParam& pi = *it;

			CParamInput* pin = new CParamInput;
			QWidget* pw = 0;
			QLineEdit* pedit; QCheckBox* pcheck;
			if (pi.type() == FE_PARAM_DOUBLE) { pin->SetWidget(pedit  = new QLineEdit); pw = pedit;  }
			if (pi.type() == FE_PARAM_BOOL) { pin->SetWidget(pcheck = new QCheckBox); pw = pcheck; }
			pin->SetParameter(pi.name(), pi.paramValue());
			assert(pw);
			pw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

			// add it to the row
			pf->addRow(pi.name(), pw);

			m_dlg->AddInputParameter(pin);
		}

		if (pg) { pg->setLayout(pf); playout->addWidget(pg); }
		else playout->addLayout(pf);
	}
	else
	{
		printf("ERROR: Cannot find property: %s\n", tag.szvalue());
	}
	++tag;
}

void UIBuilder::parseInput(XMLTag& tag, QBoxLayout* playout)
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

	string paramName = "";
	FEParamValue val;
	if (tag.isleaf())
	{
		paramName = tag.szvalue();
		ParamString ps(tag.szvalue());
		val = m_fem->FindParameter(ps);
		++tag;
	}
	else
	{
		++tag;
		do
		{
			if (tag == "param")
			{
				paramName = tag.szvalue();
				ParamString ps(tag.szvalue());
				val = m_fem->FindParameter(ps);
				++tag;
			}
			else xml.SkipTag(tag);
		}
		while (!tag.isend());
	}
	if (val.isValid() == false) 
	{
		printf("ERROR: Failed finding parameter %s\n", paramName.c_str());
		return;
	}

	QBoxLayout* pl = 0;

	if ((nalign==CParamInput::ALIGN_LEFT)||(nalign==CParamInput::ALIGN_RIGHT)) pl = new QHBoxLayout;
	else pl = new QVBoxLayout;

	QLabel* plabel = new QLabel(sz);
//	plabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	CParamInput* pi = new CParamInput;
	QWidget* pw = 0;
	QLineEdit* pedit; QCheckBox* pcheck;
	if (val.type() == FE_PARAM_DOUBLE) { pi->SetWidget(pedit = new QLineEdit); pw = pedit; }
	if (val.type() == FE_PARAM_BOOL) { pi->SetWidget(pcheck = new QCheckBox); pw = pcheck; }
	if (val.isValid()) pi->SetParameter(paramName, val);
	assert(pw);
	pw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	playout->addLayout(pl);

	switch (nalign)
	{
	case CParamInput::ALIGN_LEFT:
		pl->addWidget(plabel);
		pl->addWidget(pw);
		pl->addStretch();
		break;
	case CParamInput::ALIGN_RIGHT:
		pl->addWidget(pw);
		pl->addWidget(plabel);
		pl->addStretch();
		break;
	case CParamInput::ALIGN_TOP:
		plabel->setAlignment(Qt::AlignHCenter);
		pl->addWidget(plabel);
		pl->addWidget(pw);
		break;
	case CParamInput::ALIGN_BOTTOM:
		pl->addWidget(pw);
		plabel->setAlignment(Qt::AlignHCenter);
		pl->addWidget(plabel);
		break;
	case CParamInput::ALIGN_TOP_LEFT:
		pl->addWidget(plabel);
		pl->addWidget(pw);
		break;
	case CParamInput::ALIGN_TOP_RIGHT:
		plabel->setAlignment(Qt::AlignRight);
		pl->addWidget(plabel);
		pl->addWidget(pw);
		break;
	case CParamInput::ALIGN_BOTTOM_LEFT:
		pl->addWidget(pw);
		pl->addWidget(plabel);
		break;
	case CParamInput::ALIGN_BOTTOM_RIGHT:
		plabel->setAlignment(Qt::AlignRight);
		pl->addWidget(pw);
		pl->addWidget(plabel);
		break;
	default:
		assert(false);
	}

	m_dlg->AddInputParameter(pi);
}
