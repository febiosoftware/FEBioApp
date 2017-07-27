#include "stdafx.h"
#include "ParamInput.h"
#include <QLineEdit>
#include <QCheckBox>

//-----------------------------------------------------------------------------
CParamInput::CParamInput()
{
	m_pedit = 0;
	m_pcheck = 0;
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
			if (m_pedit) m_pedit->setText(QString::number(val.value<double>()));
		}
		if (val.type() == FE_PARAM_BOOL)
		{
			if (m_pcheck) m_pcheck->setChecked(val.value<bool>());
		}
	}
}

//-----------------------------------------------------------------------------
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
