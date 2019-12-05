#pragma once
#include <FEBioAppLib/FEBioParam.h>
#include <string>
#include <QtCore/QVariant>
#include <QWidget>
using namespace std;

class QLineEdit;
class QCheckBox;
class QSlider;

//-----------------------------------------------------------------------------
class CFloatSlider : public QWidget
{
	Q_OBJECT

public:
	CFloatSlider(QWidget* parent = 0);

	void setFloatRange(double minValue, double maxValue, double steps);

	void setFloatValue(double v);

	double getFloatValue() const;

signals:
	void valueChanged(double newVal);

protected slots:
	void onValueChanged();

private:
	double	m_minVal, m_maxVal, m_valStep;

	QSlider*	m_slider;
	QLineEdit*	m_edit;
};

//-----------------------------------------------------------------------------
//! This class connects an FEBio model parameter to an input field.
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
	void SetWidget(CFloatSlider* pw) { m_slider = pw; }

	void SetParameter(const FEBioParam& val);

	// update the parameter based on the UI value
	void UpdateParameter();

	// reset UI to initial value
	void ResetParameter();

private:
	QLineEdit*		m_pedit;
	QCheckBox*		m_pcheck;
	CFloatSlider*	m_slider;

	FEBioParam		m_val;
	QVariant		m_initVal;
};
