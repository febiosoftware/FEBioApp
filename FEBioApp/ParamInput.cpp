#include "stdafx.h"
#include "ParamInput.h"
#include <QLineEdit>
#include <QCheckBox>

//-----------------------------------------------------------------------------
CFloatSlider::CFloatSlider(QWidget* parent) : QSlider(Qt::Horizontal, parent)
{
	setFloatRange(0.0, 1.0, 0.1);
}

void CFloatSlider::setFloatRange(double minValue, double maxValue, double step)
{
	m_minVal = minValue;
	m_maxVal = maxValue;
	m_valStep = step;

	int nsteps = (maxValue - minValue) / step;

	setRange(0, nsteps);
}

void CFloatSlider::setFloatValue(double v)
{
	if (v < m_minVal) v = m_minVal;
	if (v > m_maxVal) v = m_maxVal;
	int n = (int)((v - m_minVal) / m_valStep);
	setSliderPosition(n);
}

double CFloatSlider::getFloatValue() const
{
	int n = sliderPosition();
	double f = m_minVal + n*(m_maxVal - m_minVal) / maximum();
	return f;
}

//-----------------------------------------------------------------------------
CParamInput::CParamInput()
{
	m_pedit = 0;
	m_pcheck = 0;
	m_slider = 0;
}

//-----------------------------------------------------------------------------
void CParamInput::SetParameter(const string& name, const FEParamValue& val)
{
	m_name = name;
	m_val = val;
	if (val.isValid())
	{
		if (val.type() == FE_PARAM_DOUBLE)
		{
			double d = val.value<double>();
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
		else if (val.type() == FE_PARAM_BOOL)
		{
			bool b = val.value<bool>();
			if (m_pcheck) m_pcheck->setChecked(b);
			m_initVal = b;
		}
		else if (val.type() == FE_PARAM_INT)
		{
			int n = val.value<int>();
			if (m_pedit) m_pedit->setText(QString::number(n));
			m_initVal = n;
		}
	}
}

//-----------------------------------------------------------------------------
void CParamInput::UpdateParameter()
{
	if (m_val.isValid())
	{
		if (m_val.type() == FE_PARAM_DOUBLE)
		{
			if (m_pedit)
			{
				QString s = m_pedit->text();
				double f = s.toDouble();
				m_val.value<double>() = f;
				printf("Setting parameter %s to %lg\n", m_name.c_str(), f);
			}
			else if (m_slider)
			{
				double f = m_slider->getFloatValue();
				m_val.value<double>() = f;
				printf("Setting parameter %s to %lg\n", m_name.c_str(), f);
			}
		}
		else if ((m_val.type() == FE_PARAM_BOOL) && m_pcheck)
		{
			bool b = m_pcheck->isChecked();
			m_val.value<bool>() = b;
			printf("Setting parameter %s to %s\n", m_name.c_str(), (b ? "true" : "false"));
		}
		if ((m_val.type() == FE_PARAM_INT) && m_pedit)
		{
			QString s = m_pedit->text();
			int n = s.toInt();
			m_val.value<int>() = n;
			printf("Setting parameter %s to %d\n", m_name.c_str(), n);
		}
	}
}

//-----------------------------------------------------------------------------
void CParamInput::ResetParameter()
{
	if (m_val.isValid())
	{
		if (m_val.type() == FE_PARAM_DOUBLE)
		{
			if (m_pedit) m_pedit->setText(QString::number(m_initVal.toDouble()));
		}
		else if (m_val.type() == FE_PARAM_BOOL)
		{
			if (m_pcheck) m_pcheck->setChecked(m_initVal.toBool());
		}
		else if (m_val.type() == FE_PARAM_INT)
		{
			if (m_pedit) m_pedit->setText(QString::number(m_initVal.toInt()));
		}
	}
}
