#include "FEBioParam.h"
#include "FEBioData.h"
#include <FECore/FEParam.h>

class FEBioParam::Imp
{
public:
	std::string		m_paramName;
	FEParamValue	m_paramVal;
};

FEBioParam::FEBioParam() : im(new FEBioParam::Imp)
{
}

FEBioParam::FEBioParam(const FEBioParam& fp) : im(new FEBioParam::Imp)
{
	im->m_paramName = fp.im->m_paramName;
	im->m_paramVal = fp.im->m_paramVal;
}

void FEBioParam::operator = (const FEBioParam& fp)
{
	im->m_paramName = fp.im->m_paramName;
	im->m_paramVal = fp.im->m_paramVal;
}

FEBioParam::~FEBioParam()
{
	delete im;
}

const std::string& FEBioParam::Name() const
{
	return im->m_paramName;
}

bool FEBioParam::IsValid() const
{
	return im->m_paramVal.isValid();
}

void FEBioParam::SetParameter(const std::string& name, const FEParamValue& val)
{
	im->m_paramName = name;
	im->m_paramVal = val;
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
	assert(IsType(TYPE_DOUBLE));
	return im->m_paramVal.value<double>();
}

int FEBioParam::GetInt() const
{
	assert(IsType(TYPE_INT));
	return im->m_paramVal.value<int>();
}

bool FEBioParam::GetBool() const
{
	assert(IsType(TYPE_BOOL));
	return im->m_paramVal.value<bool>();
}

void FEBioParam::SetDouble(double v)
{
	assert(IsType(TYPE_DOUBLE));
	im->m_paramVal.value<double>() = v;
}

void FEBioParam::SetInt(int v)
{
	assert(IsType(TYPE_INT));
	im->m_paramVal.value<int>() = v;
}

void FEBioParam::SetBool(bool v)
{
	assert(IsType(TYPE_BOOL));
	im->m_paramVal.value<bool>() = v;
}
