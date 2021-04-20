#pragma once
#include "febioapp_api.h"
#include <vector>

class FEBioData;
class FEBioParam;
class FELogElemData;
class FEElement;

// Base class for retrieving (scalar) data from a FEModel
class FEBIO_APP_API FEModelValuator
{
public:
	FEModelValuator();
	virtual ~FEModelValuator();

	virtual void Clear() { m_val.clear(); }

	virtual void Update() = 0;

	int Values() const { return (int)m_val.size(); }

	double GetValue(int n) { return m_val[n]; }

protected:
	std::vector<double>	m_val;	// array of data values extracted during model run
};

class FEBIO_APP_API FENodeDataValuator : public FEModelValuator
{
	class Imp;

public:
	FENodeDataValuator(FEBioData* feb);

	bool SetNodeData(const char* szdata, const char* szset);

	void Update() override;

private:
	Imp*	im;
};

class FEBIO_APP_API FEElemDataValuator : public FEModelValuator
{
	class Imp;

public:
	FEElemDataValuator(FEBioData* feb);

	void SetElementData(FELogElemData* data, FEElement* el);

	void Update() override;

private:
	Imp*	im;
};


class FEBIO_APP_API FEParamValuator : public FEModelValuator
{
	class Imp;
public:
	FEParamValuator();

	void SetParameter(FEBioParam& param);

	void Update() override;

private:
	Imp*	im;
};
