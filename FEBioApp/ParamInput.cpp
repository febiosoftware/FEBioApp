#include "stdafx.h"
#include "ParamInput.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QBoxLayout>
#include <QSlider>
#include <QValidator>

//-----------------------------------------------------------------------------
CFloatSlider::CFloatSlider(QWidget* parent) : QWidget(parent)
{
	m_slider = new QSlider(Qt::Horizontal);
	m_edit = new QLineEdit; m_edit->setValidator(new QDoubleValidator);
	m_edit->setMaximumWidth(40);
	m_edit->setReadOnly(true);
	QHBoxLayout* l = new QHBoxLayout;
	l->setMargin(0);
	l->addWidget(m_slider);
	l->addWidget(m_edit);
	setLayout(l);

	setFloatRange(0.0, 1.0, 0.1);

	QObject::connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged()));
}

void CFloatSlider::setFloatRange(double minValue, double maxValue, double step)
{
	m_minVal = minValue;
	m_maxVal = maxValue;
	m_valStep = step;

	int nsteps = (maxValue - minValue) / step;

	m_slider->setRange(0, nsteps);
}

void CFloatSlider::setFloatValue(double v)
{
	if (v < m_minVal) v = m_minVal;
	if (v > m_maxVal) v = m_maxVal;
	int n = (int)((v - m_minVal) / m_valStep);
	m_slider->setSliderPosition(n);

	v = getFloatValue();
	m_edit->setText(QString::number(v));
}

double CFloatSlider::getFloatValue() const
{
	int n = m_slider->sliderPosition();
	double f = m_minVal + n*(m_maxVal - m_minVal) / m_slider->maximum();
	return f;
}

void CFloatSlider::onValueChanged()
{
	double v = getFloatValue();
	m_edit->setText(QString::number(v));

	emit valueChanged(v);
}

//-----------------------------------------------------------------------------
CParamInput::CParamInput()
{
	m_pedit = 0;
	m_pcheck = 0;
	m_slider = 0;
}

//-----------------------------------------------------------------------------
void CParamInput::SetWidget(QLineEdit* pw)
{ 
	m_pedit = pw; 
	QObject::connect(pw, SIGNAL(editingFinished()), this, SLOT(onChanged()));
}

void CParamInput::SetWidget(QCheckBox* pw)
{ 
	m_pcheck = pw; 
	QObject::connect(pw, SIGNAL(clicked(bool)), this, SLOT(onChanged()));
}

void CParamInput::SetWidget(CFloatSlider* pw)
{ 
	m_slider = pw; 
	QObject::connect(pw, SIGNAL(valueChanged(double)), this, SLOT(onChanged()));
}

void CParamInput::onChanged()
{
	UpdateParameter();
}

//-----------------------------------------------------------------------------
void CParamInput::SetParameter(const FEBioParam& val)
{
	m_val = val;
	if (val.IsValid())
	{
		if (val.IsType(FEBioParam::TYPE_DOUBLE))
		{
			double d = val.GetDouble();
			if (m_pedit)
			{
				m_pedit->setText(QString::number(d));
				m_initVal = d;
			}
			else if (m_slider)
			{
				m_slider->setFloatValue(d);
				m_initVal = m_slider->getFloatValue();
			}
		}
		else if (val.IsType(FEBioParam::TYPE_BOOL))
		{
			bool b = val.GetBool();
			if (m_pcheck) m_pcheck->setChecked(b);
			m_initVal = b;
		}
		else if (val.IsType(FEBioParam::TYPE_INT))
		{
			int n = val.GetInt();
			if (m_pedit) m_pedit->setText(QString::number(n));
			m_initVal = n;
		}
	}
}

//-----------------------------------------------------------------------------
void CParamInput::UpdateParameter()
{
	if (m_val.IsValid())
	{
		const char* szname = m_val.Name().c_str();
		if (m_val.IsType(FEBioParam::TYPE_DOUBLE))
		{
			if (m_pedit)
			{
				QString s = m_pedit->text();
				double f = s.toDouble();
				m_val.SetDouble(f);
				printf("Setting parameter %s to %lg\n", szname, f);
			}
			else if (m_slider)
			{
				double f = m_slider->getFloatValue();
				m_val.SetDouble(f);
				printf("Setting parameter %s to %lg\n", szname, f);
			}
		}
		else if ((m_val.IsType(FEBioParam::TYPE_BOOL)) && m_pcheck)
		{
			bool b = m_pcheck->isChecked();
			m_val.SetBool(b);
			printf("Setting parameter %s to %s\n", szname, (b ? "true" : "false"));
		}
		if ((m_val.IsType(FEBioParam::TYPE_INT)) && m_pedit)
		{
			QString s = m_pedit->text();
			int n = s.toInt();
			m_val.SetInt(n);
			printf("Setting parameter %s to %d\n", szname, n);
		}
	}
}

//-----------------------------------------------------------------------------
void CParamInput::ResetParameter()
{
	if (m_val.IsValid())
	{
		if (m_val.IsType(FEBioParam::TYPE_DOUBLE))
		{
			if (m_pedit) m_pedit->setText(QString::number(m_initVal.toDouble()));
		}
		else if (m_val.IsType(FEBioParam::TYPE_BOOL))
		{
			if (m_pcheck) m_pcheck->setChecked(m_initVal.toBool());
		}
		else if (m_val.IsType(FEBioParam::TYPE_INT))
		{
			if (m_pedit) m_pedit->setText(QString::number(m_initVal.toInt()));
		}
	}
}
