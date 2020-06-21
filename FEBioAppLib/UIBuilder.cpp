#include "UIBuilder.h"
#include <QDialog>
#include <FEBioXML/XMLReader.h>
#include <FEBioMech/FEElasticMaterial.h>
#include <FECore/FESurfaceLoad.h>
#include <FECore/FEParam.h>
#include <FECore/ParamString.h>
#include <FECore/FEModel.h>
#include <FEBioLib/FEBioModel.h>
#include <FECore/FECoreTask.h>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QValidator>
//#include "PlotWidget.h"
//#include "QGLView.h"
//#include "ParamInput.h"
//#include "DataPlot.h"
#include "MyDialog.h"

UIBuilder::UIBuilder()
{
	m_data = 0;
	m_dlg = 0;
}

bool UIBuilder::BuildUI(MyDialog* dlg, ModelData& data, const char* szfile)
{
	if (dlg == 0) return false;

	XMLReader xml;
	if (xml.Open(szfile) == false) return false;

	m_dlg = dlg;
	m_data = &data;

	strcpy(m_szfile, szfile);

	XMLTag tag;
	if (xml.FindTag("febio_app", tag) == false) return false;

	++tag;
	do
	{
		if      (tag == "Model") { if (parseModel(tag) == false) return false; }
		else if (tag == "GUI"  ) { if (parseGUI  (tag) == false) return false; }
		else xml.SkipTag(tag);

		++tag;
	}
	while (!tag.isend());

	xml.Close();

	return true;
}

bool UIBuilder::parseModel(XMLTag& tag)
{
	FEBioModel& fem = m_data->m_fem;

	XMLReader& xml = *tag.m_preader;
	++tag;
	do
	{
		if (tag == "file")
		{
			strcpy(m_szfile, tag.szvalue());

			// Try to load the FE model file
			if (fem.Input(m_szfile))
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
		else if (tag == "task")
		{
			const char* sztype = tag.AttributeValue("type");
			if (sztype == 0) return false;

			m_data->m_task = fecore_new<FECoreTask>(FETASK_ID, sztype, &m_data->m_fem);
			if (m_data->m_task == 0) return false;

			m_data->m_taskFile = tag.szvalue();

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

		++tag;
	}
	while (!tag.isend());

	return true;
}

void UIBuilder::parseStretch(XMLTag& tag, QBoxLayout* playout)
{
	playout->addStretch();
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
}

int getAction(const char* szaction)
{
	int nact = -1;
	if      (strcmp(szaction, "fem.solve()"   ) == 0) nact = 0;
	else if (strcmp(szaction, "app.quit()"    ) == 0) nact = 1;
	else if (strcmp(szaction, "app.reset()"   ) == 0) nact = 2;
	else if (strcmp(szaction, "task.run()"    ) == 0) nact = 3;
	else if (strcmp(szaction, "fem.stop()"    ) == 0) nact = 4;
	else if (strcmp(szaction, "fem.pause()"   ) == 0) nact = 5;
	else if (strcmp(szaction, "fem.continue()") == 0) nact = 6;
	else printf("ERROR: Do not understand action %s\n", szaction);

	return nact;
}

void UIBuilder::parseButton(XMLTag& tag, QBoxLayout* playout)
{
	int index = 0;
	int nact[2] = {-1, -1};
	char sz[2][256] = {0};

	const char* sztitle = tag.AttributeValue("title", true);
	if (sztitle) strcpy(sz[0], sztitle);

	XMLReader& xml = *tag.m_preader;

	if (!tag.isempty())
	{
		++tag;
		do
		{
			if (tag == "action")
			{
				const char* sza = tag.szvalue();
				int naction = getAction(sza);
				nact[index] = naction;

				const char* szt = tag.AttributeValue("title", true);
				if (szt) strcpy(sz[index], szt);

				index++;

				++tag;
			}
			else xml.SkipTag(tag);
		}
		while (!tag.isend());
	}

	QHBoxLayout* pl = new QHBoxLayout;
	CActionButton* pb = new CActionButton();

	if (nact[0] != -1) pb->setAction(nact[0], sz[0], 0);
	if (nact[1] != -1) pb->setAction(nact[1], sz[1], 1);

	playout->addLayout(pl);
	pl->addStretch();
	pl->addWidget(pb);

	QObject::connect(pb, SIGNAL(doAction(int)), m_dlg, SLOT(doAction(int)));
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
}

void UIBuilder::parseGraph(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztitle = tag.AttributeValue("title");

	// size of graph
	int size[2] = {400, 400};

	XMLReader& xml = *tag.m_preader;

	int nplt = 0;

	FEParamValue xparam, yparam;

	FEBioModel& fem = m_data->m_fem;

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

				const char* sztype = tag.AttributeValue("type", true);
				if (sztype == 0)
				{
					++tag;
					do
					{
						if (tag == "x")
						{
							ParamString x_str(tag.szvalue());
							xparam = fem.GetParameterValue(x_str);
							if (xparam.isValid() == false)
							{
								printf("Failed to find parameter: %s\n", tag.szvalue());
							}
						}
						else if (tag == "y")
						{
							ParamString y_str(tag.szvalue());
							yparam = fem.GetParameterValue(y_str);
							if (yparam.isValid() == false)
							{
								printf("Failed to find parameter: %s\n", tag.szvalue());
							}
						}
						++tag;
					}
					while (!tag.isend());

					CParamDataSource* src = new CParamDataSource;
					src->m_x = xparam;
					src->m_y = yparam;

					pg->AddData(src, szname);

					xml.SkipTag(tag);
				}
				else if (strcmp(sztype, "static") == 0)
				{
					CStaticDataSource* src = new CStaticDataSource;
					++tag;
					do
					{
						double p[2];
						tag.value(p, 2);

						src->m_data.push_back(QPointF(p[0], p[1]));
						++tag;
					}
					while (!tag.isend());

					pg->AddData(src, szname);

					++tag;
				}
			}
			else xml.SkipTag(tag);
		}
		while (!tag.isend());
	}

	pg->setMinimumSize(QSize(size[0], size[1]));

	playout->addWidget(pg, 1);
	m_dlg->AddGraph(pg);
}

void UIBuilder::parsePlot3d(XMLTag& tag, QBoxLayout* playout)
{
	char sz[256] = {0};
	strcpy(sz, tag.AttributeValue("title"));

	// size of plot view
	int size[2] = {400, 400};

	bool brange = false;
	double rng[2];
	const char* szmap = "displacement";
	XMLReader& xml = *tag.m_preader;
	double bgc[3] = {0.8, 0.8, 1.0};
	double fgc[3] = {0.0, 0.0, 0.0 };
	double w[3] = { 0, 0, 0 };
	double smoothingAngle = 60.0;
	int timeFormat = 0;
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
			else if (tag == "bg_color")
			{
				tag.value(bgc, 3);
				++tag;
			}
			else if (tag == "fg_color")
			{
				tag.value(fgc, 3);
				++tag;
			}
			else if (tag == "map")
			{
				const char* sztype = tag.AttributeValue("type");
				if (sztype) szmap = sztype;

				if (tag.isleaf()) ++tag;
				else
				{
					++tag;
					do
					{
						if (tag == "range")
						{
							brange = true;
							tag.value(rng, 2);							
						}
						++tag;
					}
					while (!tag.isend());
					++tag;
				}
			}
			else if (tag == "rotation")
			{
				tag.value(w, 3);
				++tag;
			}
			else if (tag == "smoothing_angle")
			{
				tag.value(smoothingAngle);
				++tag;
			}
			else if (tag == "time_format")
			{
				tag.value(timeFormat);
				++tag;
			}
			else xml.SkipTag(tag);
		}
		while (!tag.isend());
	}

	QGLView* pgl = new QGLView(0, size[0], size[1]);
	playout->addWidget(pgl);

	pgl->SetTimeFormat(timeFormat);
	pgl->SetSmoothingAngle(smoothingAngle);	// must be set before SetFEModel is called
	pgl->SetFEModel(&m_data->m_fem);
	pgl->SetDataSource(szmap);
	pgl->SetRotation(w[0], w[1], w[2]);
	if (brange) pgl->SetDataRange(rng[0], rng[1]);
	pgl->SetBackgroundColor(bgc[0], bgc[1], bgc[2]);
	pgl->SetForegroundColor(fgc[0], fgc[1], fgc[2]);
	m_dlg->AddPlot3D(pgl);
}

void UIBuilder::parseInputList(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztitle = tag.AttributeValue("title", true);

	int naction = -1;

	FEModel& fem = m_data->m_fem;

	FECoreBase* pc = 0;
	if (tag.isleaf())
	{
		ParamString ps(tag.szvalue());
		pc = fem.FindComponent(ps);
	}
	else
	{
		++tag;
		do
		{
			if (tag == "params")
			{
				ParamString ps(tag.szvalue());
				pc = fem.FindComponent(ps);
			}
			else if (tag == "action")
			{
				const char* szaction = tag.szvalue();
				if (strcmp(szaction, "fem.solve()") == 0) naction = 0;
			}
			
			++tag;
		}
		while (!tag.isend());
	}

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

			if (pi.dim() == 1)
			{
				FEParamValue val = pi.paramValue();
				CParamInput* pin = nullptr;
				QWidget* pw = nullptr;
				QLineEdit* pedit; QCheckBox* pcheck;
				switch (val.type())
				{
				case FE_PARAM_DOUBLE:
				{
					pin = new CParamInput;
					pin->SetWidget(pedit = new QLineEdit);
					pw = pedit;
					if (naction == 0) QObject::connect(pedit, SIGNAL(editingFinished()), m_dlg, SLOT(Run()));
				}
				break;
				case FE_PARAM_BOOL:
				{
					pin = new CParamInput;
					pin->SetWidget(pcheck = new QCheckBox);
					pw = pcheck;
					if (naction == 0) QObject::connect(pcheck, SIGNAL(stateChanged(int)), m_dlg, SLOT(Run()));
				}
				break;
				case FE_PARAM_INT:
				{
					pin = new CParamInput;
					pin->SetWidget(pedit = new QLineEdit);
					pw = pedit;
					if (naction == 0) QObject::connect(pedit, SIGNAL(editingFinished()), m_dlg, SLOT(Run()));
				}
				break;
				}

				if (pin)
				{
					assert(pw);
					pin->SetParameter(pi.name(), val);
					pw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

					// add it to the row
					pf->addRow(pi.name(), pw);
					m_dlg->AddInputParameter(pin);
				}
			}
		}

		if (pg) { pg->setLayout(pf); playout->addWidget(pg); }
		else playout->addLayout(pf);
	}
	else
	{
		printf("ERROR: Cannot find property: %s\n", tag.szvalue());
	}
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

	FEModel& fem = m_data->m_fem;

	XMLReader& xml = *tag.m_preader;

	string paramLabel;
	string paramName = "";
	FEParamValue val;
	bool brange = false;
	double rng[3];
	if (tag.isleaf())
	{
		paramName = tag.szvalue();
		ParamString ps(tag.szvalue());
		val = fem.GetParameterValue(ps);
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
				val = fem.GetParameterValue(ps);
				++tag;
			}
			else if (tag == "range")
			{
				tag.value(rng, 3);
				brange = true;
				++tag;
			}
			else if (tag == "label")
			{
				paramLabel = tag.szvalue();
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

	if (paramLabel.empty()) paramLabel = sz;

	QBoxLayout* pl = 0;

	if ((nalign==CParamInput::ALIGN_LEFT)||(nalign==CParamInput::ALIGN_RIGHT)) pl = new QHBoxLayout;
	else pl = new QVBoxLayout;

	QLabel* plabel = new QLabel(QString::fromStdString(paramLabel));
//	plabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	CParamInput* pi = new CParamInput;
	QWidget* pw = 0;
	if (val.type() == FE_PARAM_DOUBLE)
	{ 
		if (brange)
		{
			CFloatSlider* slider = new CFloatSlider;
			slider->setFloatRange(rng[0], rng[1], rng[2]);
			pw = slider;
			pi->SetWidget(slider);
			QObject::connect(slider, SIGNAL(valueChanged(double)), m_dlg, SLOT(paramChanged()));
		}
		else
		{
			QLineEdit* edit = new QLineEdit; 
			edit->setValidator(new QDoubleValidator);
			pi->SetWidget(edit);
			pw = edit; 
			QObject::connect(edit, SIGNAL(textEdited(const QString&)), m_dlg, SLOT(paramChanged()));
		}
	}
	if (val.type() == FE_PARAM_BOOL  )
	{ 
		QCheckBox* pcheck = new QCheckBox;
		pi->SetWidget(pcheck);
		pw = pcheck; 
	}
	if (val.type() == FE_PARAM_INT)
	{ 
		QLineEdit* pedit = new QLineEdit; pedit->setValidator(new QIntValidator);
		pi->SetWidget(pedit); pw = pedit; 
	}
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
