#pragma once
#include "febioapp_api.h"

class FEBioData;
class FEBioParam;

// Base class for retrieving (scalar) data from a FEModel
class FEBIO_APP_API FEModelValuator
{
public:
	FEModelValuator();
	virtual ~FEModelValuator();

	virtual double GetValue() = 0;
};

class FEBIO_APP_API FENodeDataValuator : public FEModelValuator
{
	class Imp;

public:
	FENodeDataValuator(FEBioData* feb);

	bool SetNodeData(const char* szdata, const char* szset);

	double GetValue() override;

private:
	Imp*	im;
};

class FEBIO_APP_API FEParamValuator : public FEModelValuator
{
	class Imp;
public:
	FEParamValuator();

	void SetParameter(FEBioParam& param);

	double GetValue() override;

private:
	Imp*	im;
};
