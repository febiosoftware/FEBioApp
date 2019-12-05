#pragma once
#include <QtCore/QObject>
#include "FEBioParam.h"

namespace FEBioApp {
	class GLMesh;
}

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
	bool HasTask() const;

	bool InitModel();

	bool ResetModel();

	bool RunModel();

	bool SolveModel();

	bool ReadFEBioFile(const char* szfile);

	bool SetFEBioTask(const char* sztaskName, const char* sztaskFile);

	FEBioApp::GLMesh* BuildGLMesh();

	void UpdateGLMesh(FEBioApp::GLMesh* mesh, const std::string& map);

	double GetSimulationTime() const;

	int GetFEBioStatus() const;

	void SetFEBioStatus(FEBIO_STATUS s);

	void GetDataRange(double rng[2]);

public:
	bool FEBioCallback(unsigned int nwhen);

	FEBioParam	GetFEBioParameter(const std::string& paramName);

	std::vector<FEBioParam>	GetFEBioParameterList(const std::string& name);

signals:
	void modelInit();
	void timeStepDone();

private:
	Imp*	im;
};
