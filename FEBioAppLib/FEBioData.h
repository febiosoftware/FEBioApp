#pragma once
#include <QtCore/QObject>
#include "FEBioParam.h"

namespace FEBioApp {
	class GLMesh;
}

#define FEMODEL_PTR void*

//------------------------------------------------------------------
// Class that manages the FEBio model data and interface
// for interacting with FEBio
class FEBIO_APP_API FEBioData : public QObject
{
	Q_OBJECT

	class Imp;

public:
	enum FEBIO_STATUS
	{
		RUNNING,
		PAUSED,
		STOPPED
	};

public:
	FEBioData();
	~FEBioData();

	static bool InitFEBio();

public:
	FEBioApp::GLMesh* BuildGLMesh(int modelIndex);

	void UpdateGLMesh(FEBioApp::GLMesh* mesh, const std::string& map);

public:
	bool AddModel(const std::string& modelId, const std::string& fileName, const char* sztask = nullptr);

	int Models();

	int GetModelIndex(const std::string& modelId);

	void SetModelId(int index, const std::string& id);

	std::string GetModelId(int i);

	std::string GetModelFile(int i);

	bool IsModelInitialized(int i);

	bool InitModel(int index);

	bool ResetModel(int index);

	bool SolveModel(int index);

	void StopModel(int index);

	bool HasTask(int index) const;

	int GetFEBioStatus(int index) const;

	void StopAll();

	double GetSimulationTime(int index) const;

	void GetDataRange(int index, double rng[2]);

	bool ForceStop(int index);

public:
	void FEBioCallback(int modelIndex, unsigned int nwhen);

	FEBioParam	GetFEBioParameter(const std::string& paramName);

	std::vector<FEBioParam>	GetFEBioParameterList(const std::string& name);

	FEMODEL_PTR GetFEModel();

signals:
	void modelInit(int index);
	void timeStepDone(int index);
	void modelReset(int index);

private:
	Imp&	im;
};
