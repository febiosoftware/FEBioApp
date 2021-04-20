#pragma once
#include <string>
#include "febioapp_api.h"
class FEParamValue;
class FEModel;

class FEBIO_APP_API FEBioParam
{
	class Imp;

public:
	enum Type
	{
		TYPE_DOUBLE,
		TYPE_BOOL,
		TYPE_INT
	};

public:
	FEBioParam();
	FEBioParam(const FEBioParam& fp);
	void operator = (const FEBioParam& fp);

	const std::string& Name() const;

	~FEBioParam();

	bool IsValid() const;

	bool IsType(Type type) const;

	void SetParameter(const std::string& name, FEModel* fem);
	void SetParameter(const std::string& name, const FEParamValue& param);

	double GetDouble() const;
	int GetInt() const;
	bool GetBool() const;

	void SetDouble(double v);
	void SetInt(int v);
	void SetBool(bool v);

	FEModel* GetFEModel();

private:
	Imp*	im;
};
