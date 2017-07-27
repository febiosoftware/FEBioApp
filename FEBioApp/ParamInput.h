#pragma once
#include <FECore/FEParam.h>
#include <string>
#include <QtCore/QVariant>
using namespace std;

class QLineEdit;
class QCheckBox;

//-----------------------------------------------------------------------------
//! This class connects an FE model parameter to an input field.
class CParamInput
{
public:
	enum {
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_TOP,
		ALIGN_BOTTOM,
		ALIGN_TOP_LEFT,
		ALIGN_TOP_RIGHT,
		ALIGN_BOTTOM_LEFT,
		ALIGN_BOTTOM_RIGHT
	};

public:
	CParamInput();

	void SetWidget(QLineEdit* pw) { m_pedit = pw; }
	void SetWidget(QCheckBox* pw) { m_pcheck = pw; }

	void SetParameter(const string& name, const FEParamValue& val);

	// update the parameter based on the UI value
	void UpdateParameter();

	// reset UI to initial value
	void ResetParameter();

private:
	QLineEdit*	m_pedit;
	QCheckBox*	m_pcheck;

	FEParamValue	m_val;
	string			m_name;
	QVariant		m_initVal;
};
