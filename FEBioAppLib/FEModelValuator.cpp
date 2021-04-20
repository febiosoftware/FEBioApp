#include "FEModelValuator.h"
#include "FEBioParam.h"
#include "FEBioData.h"
#include <FEBioLib/FEBioModel.h>
#include <FECore/NodeDataRecord.h>
#include <FECore/FECoreKernel.h>
#include <FECore/ElementDataRecord.h>
#include <assert.h>

FEModelValuator::FEModelValuator()
{

}

FEModelValuator::~FEModelValuator()
{

}

//===========================================================================
class FENodeDataValuator::Imp
{
public:
	FEBioData*	m_feb;
	FENodeLogData*	m_data;
	FENodeSet*	m_set;

public:
	Imp()
	{
		m_feb = nullptr;
		m_data = nullptr;
		m_set = nullptr;
	}

	~Imp()
	{
		delete m_data;
		delete m_set;
	}
};

//---------------------------------------------------------------------------
FENodeDataValuator::FENodeDataValuator(FEBioData* feb) : im(new FENodeDataValuator::Imp)
{
	im->m_feb = feb;
}

bool FENodeDataValuator::SetNodeData(const char* szdata, const char* szset)
{
	FEBioModel* fem = (FEBioModel*)im->m_feb->GetFEModel();

	FEMesh& mesh = fem->GetMesh();
	im->m_set = mesh.FindNodeSet(szset);
	if (im->m_set == nullptr) return false;

	im->m_data = fecore_new<FENodeLogData>(szdata, fem);
	if (im->m_data == nullptr) return false;

	return true;
}

void FENodeDataValuator::Update()
{
	FEMesh* mesh = im->m_set->GetMesh();
	FENodeSet& ns = *im->m_set;
	double sum = 0.0;
	for (int i = 0; i < ns.Size(); ++i)
	{
		double vi = im->m_data->value(ns[i]);
		sum += vi;
	}
	m_val.push_back(sum);
}

//===========================================================================
class FEElemDataValuator::Imp
{
public:
	Imp() { m_data = nullptr; m_pe = nullptr; }

public:
	FEElement*	m_pe;
	FELogElemData*	m_data;
};

FEElemDataValuator::FEElemDataValuator(FEBioData* feb) : im(new FEElemDataValuator::Imp)
{

}

void FEElemDataValuator::SetElementData(FELogElemData* data, FEElement* pe)
{
	im->m_data = data;
	im->m_pe = pe;
}

void FEElemDataValuator::Update()
{
	if (im->m_data && im->m_pe)
	{
		double v = im->m_data->value(*im->m_pe);
		m_val.push_back(v);
	}
}

//===========================================================================
class FEParamValuator::Imp
{
public:
	FEBioParam	m_param;
};

//---------------------------------------------------------------------------
FEParamValuator::FEParamValuator() : im(new FEParamValuator::Imp)
{

}

void FEParamValuator::SetParameter(FEBioParam& param)
{
	im->m_param = param;
}

void FEParamValuator::Update()
{
	assert(im->m_param.IsValid());
	assert(im->m_param.IsType(FEBioParam::TYPE_DOUBLE));
	double v = im->m_param.GetDouble();
	m_val.push_back(v);
}
