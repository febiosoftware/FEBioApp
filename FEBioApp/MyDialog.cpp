#include "stdafx.h"
#include "MyDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QTabWidget>
#include <FEBioXML/XMLReader.h>
#include <FEBioMech/FEElasticMaterial.h>
#include <FECore/FESurfaceLoad.h>
#include <FECore/FEParam.h>
#include <FECore/ParamString.h>
#include "QPlotWidget.h"
#include "QGLView.h"

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

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
CParamInput::CParamInput()
{
	m_pedit = 0;
	m_pcheck = 0;
}

void CParamInput::SetParameter(const string& name, const FEParamValue& val)
{
	m_name = name;
	m_val = val;
	if (val.isValid()) 
	{
		if (val.type() == FE_PARAM_DOUBLE)
		{
			if (m_pedit) m_pedit->setText(QString::number(val.value<double>()));
		}
		if (val.type() == FE_PARAM_BOOL)
		{
			if (m_pcheck) m_pcheck->setChecked(val.value<bool>());
		}
	}
}

void CParamInput::UpdateParameter()
{
	if (m_val.isValid()) 
	{
		if ((m_val.type() == FE_PARAM_DOUBLE) && m_pedit)
		{
			QString s = m_pedit->text();
			double f = s.toDouble();
			m_val.value<double>() = f;
			printf("Setting parameter %s to %lg\n", m_name.c_str(), f);
		}
		else if ((m_val.type() == FE_PARAM_BOOL) && m_pcheck)
		{
			bool b = m_pcheck->isChecked();
			m_val.value<bool>() = b;
			printf("Setting parameter %s to %s\n", m_name.c_str(), (b ? "true" : "false"));
		}
	}
}

//-----------------------------------------------------------------------------
CDataPlot::CDataPlot(QWidget* parent) : QPlotWidget(parent)
{
	
}

//-----------------------------------------------------------------------------
void CDataPlot::AddData(CDataSource& data)
{
	m_data.push_back(data);
	QPlotData d;
	addPlotData(d);
}

//-----------------------------------------------------------------------------
void CDataPlot::Update(FEModel& fem)
{
	for (int i=0; i<plots(); ++i)
	{
		QPlotData& plot = getPlotData(i);

		CDataSource& data = m_data[i];

		if (data.m_x.isValid() && data.m_y.isValid())
			plot.addPoint(data.m_x.value<double>(), data.m_y.value<double>());
	}
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
		// update all model data
		fem.UpdateModelData();

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
	for (int i=0; i<(int) m_plot.size(); ++i) m_plot[i]->clearData();

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
				QMessageBox::critical(this, "FEBioApp", QString("Failed loading file %1").arg(m_szfile));
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

void MyDialog::parseStretch(XMLTag& tag, QBoxLayout* playout)
{
	playout->addStretch();
	++tag;
}

void MyDialog::parseGroup(XMLTag& tag, QBoxLayout* playout)
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

void MyDialog::parseVGroup(XMLTag& tag, QBoxLayout* playout)
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

void MyDialog::parseHGroup(XMLTag& tag, QBoxLayout* playout)
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

void MyDialog::parseTabGroup(XMLTag& tag, QBoxLayout* playout)
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

void MyDialog::parseButton(XMLTag& tag, QBoxLayout* playout)
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

void MyDialog::parseLabel(XMLTag& tag, QBoxLayout* playout)
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

void MyDialog::parseGraph(XMLTag& tag, QBoxLayout* playout)
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

				++tag;
				do
				{
					if (tag == "x")
					{
						ParamString x_str(tag.szvalue());
						xparam = m_fem.FindParameter(x_str);
					}
					else if (tag == "y")
					{
						ParamString y_str(tag.szvalue());
						yparam = m_fem.FindParameter(y_str);
					}
					++tag;
				}
				while (!tag.isend());

				CDataSource src;
				src.m_x = xparam;
				src.m_y = yparam;

				pg->AddData(src);

				xml.SkipTag(tag);
			}
			else xml.SkipTag(tag);
		}
		while (!tag.isend());
	}

	pg->setMinimumSize(QSize(size[0], size[1]));

	playout->addWidget(pg, 1);
	m_plot.push_back(pg);

	++tag;
}

void MyDialog::parsePlot3d (XMLTag& tag, QBoxLayout* playout)
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

	pgl->SetFEModel(&m_fem);
	m_gl.push_back(pgl);

	++tag;
}

void MyDialog::parseInputList(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztitle = tag.AttributeValue("title", true);

	ParamString ps(tag.szvalue());
	FECoreBase* pc = m_fem.FindComponent(ps);
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

			m_in.push_back(pin);
		}

		if (pg) { pg->setLayout(pf); playout->addWidget(pg); }
		else playout->addLayout(pf);
	}
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

	string paramName = "";
	FEParamValue val;
	if (tag.isleaf())
	{
		paramName = tag.szvalue();
		ParamString ps(tag.szvalue());
		val = m_fem.FindParameter(ps);
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
				val = m_fem.FindParameter(ps);
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

	m_in.push_back(pi);
					
	++tag;
}

// Include the MOC stuff
#include "moc_MyDialog.cpp"
