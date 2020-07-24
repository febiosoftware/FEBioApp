#include "stdafx.h"
#include "UIBuilder.h"
#include <QDialog>
#include <XML/XMLReader.h>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QValidator>
#include "PlotWidget.h"
#include "QGLView.h"
#include "ParamInput.h"
#include "DataPlot.h"
#include "MyDialog.h"
#include <FEBioAppLib/FEBioData.h>

UIBuilder::UIBuilder()
{
	m_data = 0;
	m_dlg = 0;
}

bool UIBuilder::BuildUI(MyDialog* dlg, FEBioData& data, const char* szfile)
{
	if (dlg == 0) return false;

	XMLReader xml;
	if (xml.Open(szfile) == false) return false;

	m_fileName = szfile;

	m_dlg = dlg;
	m_data = &data;

	XMLTag tag;
	if (xml.FindTag("febio_app", tag) == false) return false;

	try {
		++tag;
		do
		{
			if (tag == "Model") { if (parseModel(tag) == false) return false; }
			else if (tag == "GUI") { if (parseGUI(tag) == false) return false; }
			else xml.SkipTag(tag);

			++tag;
		} while (!tag.isend());
	}
	catch (...)
	{

	}

	xml.Close();

	return true;
}

bool UIBuilder::parseModel(XMLTag& tag)
{
	XMLReader& xml = *tag.m_preader;

	const char* szid = tag.AttributeValue("id", true);
	if (szid == nullptr) szid = "fem";

	const char* szfile = tag.AttributeValue("file");

	const char* sztask = tag.AttributeValue("task", true);

	assert(tag.isleaf());

	// add the new model
	bool b = m_data->AddModel(szid, szfile, sztask);

	// Try to load the FE model file
	if (b)
	{
		printf("Success loading input file %s\n", szfile);
	}
	else
	{
		printf("Failed loading input file %s\n", szfile);
		return false;
	}

	return true;
}

bool UIBuilder::parseGUI(XMLTag& tag)
{
	QBoxLayout* playout = dynamic_cast<QBoxLayout*>(m_dlg->layout());

	// read the window title
	char sz[256] = {0};
	strcpy(sz, tag.AttributeValue("title"));

	if (strcmp(sz, "$(filename)") == 0) strcmp(sz, m_fileName.c_str());
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

void UIBuilder::parseButton(XMLTag& tag, QBoxLayout* playout)
{
	int nact[2] = {-1, -1};
	char sz[2][256] = {0};

	const char* szaction = nullptr;

	assert(tag.isleaf());

	const char* sztitle = tag.AttributeValue("text", true);
	if (sztitle) strcpy(sz[0], sztitle);

	szaction = tag.AttributeValue("onClick");
		
	CActionButton* pb = new CActionButton();
	pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	if (szaction)
	{
		pb->setText(QString(sztitle));
		pb->setCode(szaction);
	}

	playout->addWidget(pb);

	QObject::connect(pb, SIGNAL(runCode(QString&)), m_dlg, SLOT(RunCode(QString&)));
}

void UIBuilder::parseLabel(XMLTag& tag, QBoxLayout* playout)
{
	string name, id;
	for (int i = 0; i < tag.m_natt; ++i)
	{
		XMLAtt& att = tag.m_att[i];

		if (strcmp(att.m_sztag, "text") == 0)
		{
			name = att.cvalue();
		}
		else if (strcmp(att.m_sztag, "id") == 0)
		{
			id = att.cvalue();
		}
	}

	QLabel* plabel = new QLabel(QString::fromStdString(name));
	if (id.empty() == false) plabel->setObjectName(QString::fromStdString(id));
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

				FEModelValuator* val_x = nullptr;
				FEModelValuator* val_y = nullptr;

				const char* sztype = tag.AttributeValue("type", true);
				if (sztype == 0)
				{
					++tag;
					do
					{
						if (tag == "x")
						{
							const char* sztype = tag.AttributeValue("type", true);
							if (sztype == nullptr) sztype = "param";

							if (strcmp(sztype, "param") == 0)
							{
								FEBioParam xparam = m_data->GetFEBioParameter(tag.szvalue());
								if (xparam.IsValid() == false)
								{
									printf("Failed to find parameter: %s\n", tag.szvalue());
								}

								FEParamValuator* vx = new FEParamValuator;
								vx->SetParameter(xparam);
								val_x = vx;
							}
						}
						else if (tag == "y")
						{
							const char* sztype = tag.AttributeValue("type", true);
							if (sztype == nullptr) sztype = "param";

							if (strcmp(sztype, "param") == 0)
							{
								FEBioParam yparam = m_data->GetFEBioParameter(tag.szvalue());
								if (yparam.IsValid() == false)
								{
									printf("Failed to find parameter: %s\n", tag.szvalue());
								}
								FEParamValuator* vy = new FEParamValuator;
								vy->SetParameter(yparam);
								val_y = vy;
							}
							else if (strcmp(sztype, "filter_sum") == 0)
							{
								++tag;
								do
								{
									if (tag == "node_data")
									{
										const char* szdata = tag.AttributeValue("data");
										const char* szset = tag.AttributeValue("node_set");

										FENodeDataValuator* val = new FENodeDataValuator(m_data);
										if (val->SetNodeData(szdata, szset) == false)
										{
											printf("Do you even know what you are doing??!!");
										}

										val_y = val;
									}
									++tag;
								} while (!tag.isend());
							}
						}
						++tag;
					}
					while (!tag.isend());

					CParamDataSource* src = new CParamDataSource;
					src->m_x = val_x;
					src->m_y = val_y;

					pg->AddData(src, szname);

					++tag;
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
	int modelId = -1;
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
			else if (tag == "model")
			{
				const char* szmodel = tag.szvalue();
				modelId = m_data->GetModelIndex(szmodel); assert(modelId >= 0);
				if (modelId < 0) throw 1;
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
			else
			{
				xml.SkipTag(tag);
				++tag;
			}
		}
		while (!tag.isend());
	}

	QGLView* pgl = new QGLView(0, size[0], size[1]);
	playout->addWidget(pgl);

	pgl->SetTimeFormat(timeFormat);
	pgl->SetSmoothingAngle(smoothingAngle);	// must be set before SetFEModel is called
	pgl->SetFEModel(m_data, modelId);
	pgl->SetDataSource(szmap);
	pgl->SetRotation(w[0], w[1], w[2]);
	if (brange) pgl->SetDataRange(rng[0], rng[1]);
	pgl->SetBackgroundColor(bgc[0], bgc[1], bgc[2]);
	pgl->SetForegroundColor(fgc[0], fgc[1], fgc[2]);
	m_dlg->AddPlot3D(pgl);
}

void UIBuilder::parseInputList(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztitle = tag.AttributeValue("text", true);

	const char* szparams = tag.AttributeValue("params");

	assert(tag.isleaf());

	int naction = -1;

	vector<FEBioParam> paramList;
	paramList = m_data->GetFEBioParameterList(szparams);

	if (!paramList.empty())
	{
		QGroupBox* pg = 0;
		if (sztitle) pg = new QGroupBox(sztitle);

		QFormLayout* pf = new QFormLayout;
		for (size_t i=0; i<paramList.size(); ++i)
		{
			FEBioParam& pi = paramList[i];

			CParamInput* pin = nullptr;
			QWidget* pw = nullptr;
			QLineEdit* pedit; QCheckBox* pcheck;

			if (pi.IsType(FEBioParam::TYPE_DOUBLE))
			{
				pin = new CParamInput;
				pin->SetWidget(pedit = new QLineEdit);
				pw = pedit;
			}

			if (pi.IsType(FEBioParam::TYPE_BOOL))
			{
				pin = new CParamInput;
				pin->SetWidget(pcheck = new QCheckBox);
				pw = pcheck;
			}

			if (pi.IsType(FEBioParam::TYPE_INT))
			{
				pin = new CParamInput;
				pin->SetWidget(pedit = new QLineEdit);
				pw = pedit;
			}

			if (pin)
			{
				assert(pw);
				pin->SetParameter(pi);
				pw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

				// add it to the row
				pf->addRow(QString::fromStdString(pi.Name()), pw);
				m_dlg->AddInputParameter(pin);
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
	// read the title
	char sz[256] = {0};
	strcpy(sz, tag.AttributeValue("text"));

	// read the align
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

	// read the parameter
	string paramName;
	const char* szparam = tag.AttributeValue("param");
	paramName = szparam;
	FEBioParam param = m_data->GetFEBioParameter(paramName);
	if (param.IsValid() == false)
	{
		printf("ERROR: Failed finding parameter %s\n", paramName.c_str());
		return;
	}

	// read the range
	double rng[3];
	bool brange = false;
	XMLAtt* rngAtt = tag.AttributePtr("range");
	if (rngAtt)
	{
		rngAtt->value(rng, 3);
		brange = true;
	}	

	assert(tag.isempty());

	QBoxLayout* pl = 0;

	if ((nalign==CParamInput::ALIGN_LEFT)||(nalign==CParamInput::ALIGN_RIGHT)) pl = new QHBoxLayout;
	else pl = new QVBoxLayout;

	QLabel* plabel = new QLabel(sz);
//	plabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	CParamInput* pi = new CParamInput;
	QWidget* pw = 0;
	if (param.IsType(FEBioParam::TYPE_DOUBLE))
	{ 
		if (brange)
		{
			CFloatSlider* slider = new CFloatSlider;
			slider->setFloatRange(rng[0], rng[1], rng[2]);
			pw = slider;
			pi->SetWidget(slider);
		}
		else
		{
			QLineEdit* edit = new QLineEdit; 
			edit->setValidator(new QDoubleValidator);
			pi->SetWidget(edit);
			pw = edit; 
		}
	}
	if (param.IsType(FEBioParam::TYPE_BOOL))
	{ 
		QCheckBox* pcheck = new QCheckBox;
		pi->SetWidget(pcheck);
		pw = pcheck; 
	}
	if (param.IsType(FEBioParam::TYPE_INT))
	{ 
		QLineEdit* pedit = new QLineEdit; pedit->setValidator(new QIntValidator);
		pi->SetWidget(pedit); pw = pedit; 
	}
	if (param.IsValid()) pi->SetParameter(param);
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
