#include "FEBioParam.h"
#include "FEBioData.h"
#include <FECore/FEParam.h>
#include <FECore/FEModel.h>

class FEBioParam::Imp
{
public:
	std::string		m_paramName;
	FEModel*		m_fem;
	FEParamValue	m_paramVal;

public:
	bool isValid() { return m_paramVal.isValid(); }
	bool init() { 
		ParamString ps(m_paramName.c_str());
		if (m_fem) m_paramVal = m_fem->GetParameterValue(ps);
		return m_paramVal.isValid();
	}
	void checkInit()
	{
		if (isValid() == false) init();
	}
};

FEBioParam::FEBioParam() : im(new FEBioParam::Imp)
{
	im->m_fem = nullptr;
}

FEBioParam::FEBioParam(const FEBioParam& fp) : im(new FEBioParam::Imp)
{
	im->m_paramName = fp.im->m_paramName;
	im->m_paramVal = fp.im->m_paramVal;
	im->m_fem = fp.im->m_fem;
}

void FEBioParam::operator = (const FEBioParam& fp)
{
	im->m_paramName = fp.im->m_paramName;
	im->m_paramVal = fp.im->m_paramVal;
	im->m_fem = fp.im->m_fem;
}

FEBioParam::~FEBioParam()
{
	delete im;
}

FEModel* FEBioParam::GetFEModel()
{
	return im->m_fem;
}

const std::string& FEBioParam::Name() const
{
	return im->m_paramName;
}

bool FEBioParam::IsValid() const
{
	im->checkInit();
	return im->m_paramVal.isValid();
}

void FEBioParam::SetParameter(const std::string& name, FEModel* fem)
{
	im->m_paramName = name;
	im->m_fem = fem;
	im->checkInit();
}

void FEBioParam::SetParameter(const std::string& name, const FEParamValue& param)
{
	im->m_paramName = name;
	im->m_fem = nullptr;
	im->m_paramVal = param;
}


bool FEBioParam::IsType(Type type) const
{
	assert(im->m_paramVal.isValid());
	if (im->m_paramVal.isValid() == false) return false;

	switch (type)
	{
	case TYPE_DOUBLE: return (im->m_paramVal.type() == FE_PARAM_DOUBLE); break;
	case TYPE_INT   : return (im->m_paramVal.type() == FE_PARAM_INT); break;
	case TYPE_BOOL  : return (im->m_paramVal.type() == FE_PARAM_BOOL); break;
	default:
		assert(false);
	}
	return false;
}

double FEBioParam::GetDouble() const
{
	im->checkInit();
	assert(IsType(TYPE_DOUBLE));
	return im->m_paramVal.value<double>();
}

int FEBioParam::GetInt() const
{
	im->checkInit();
	assert(IsType(TYPE_INT));
	return im->m_paramVal.value<int>();
}

bool FEBioParam::GetBool() const
{
	im->checkInit();
	assert(IsType(TYPE_BOOL));
	return im->m_paramVal.value<bool>();
}

void FEBioParam::SetDouble(double v)
{
	im->checkInit();
	assert(IsType(TYPE_DOUBLE));
	im->m_paramVal.value<double>() = v;
}

void FEBioParam::SetInt(int v)
{
	im->checkInit();
	assert(IsType(TYPE_INT));
	im->m_paramVal.value<int>() = v;
}

void FEBioParam::SetBool(bool v)
{
	im->checkInit();
	assert(IsType(TYPE_BOOL));
	im->m_paramVal.value<bool>() = v;
}
