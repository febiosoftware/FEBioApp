#include "FEBioData.h"
#include <FECore/FECoreTask.h>
#include <FEBioLib/FEBioModel.h>
#include <FEBioMech/FEElasticMaterial.h>
#include <FECore/FESurfaceLoad.h>
#include <FECore/FEParam.h>
#include <FECore/ParamString.h>
#include <FECore/FEModel.h>
#include <FEBioLib/FEBioModel.h>
#include <FECore/FECoreTask.h>
#include <FEBioLib/febio.h>
#include <FEBioLib/version.h>
#include "GLMesh.h"

class FEBioAppModel
{
public:
	FEBioModel		m_fem;
	std::string		m_fileName;
	FECoreTask*		m_task;
	std::string		m_taskFile;
	double			m_dataRange[2];
	FEBioData::FEBIO_STATUS	m_runStatus;
	bool			m_modelInitialized;
	bool			m_bforceStop;
	FEBioData*		m_data;
	int				m_id;

	FEBioAppModel(FEBioData* data) : m_task(0), m_data(data)
	{
		m_runStatus = FEBioData::STOPPED;
		m_dataRange[0] = 0.0;
		m_dataRange[1] = 1.0;
		m_modelInitialized = false;
		m_id = -1;
	}

	~FEBioAppModel()
	{
		if (m_task) delete m_task; m_task = 0;
	}

	bool InitModel()
	{
		if (m_task)
		{
			return m_task->Init(m_taskFile.c_str());
		}
		else
		{
			m_fem.SetLogLevel(0);
			if (m_fem.Init() == false) return false;
		}
		return true;
	}

	bool Solve()
	{
		if (m_task)
		{
			return m_task->Run();
		}
		else
		{
			return m_fem.Solve();
		}
	}

	bool febio_cb(unsigned int nwhen)
	{
		m_data->FEBioCallback(m_id, nwhen);
		if (nwhen != CB_INIT)
		{
			if (m_runStatus == FEBioData::STOPPED) return false;
		}
		return true;
	}
};

bool febio_cb(FEModel* pfem, unsigned int nwhen, void* pd)
{
	FEBioAppModel* pThis = (FEBioAppModel*)pd;
	return pThis->febio_cb(nwhen);
}


//-----------------------------------------------------------------------------
class FEBioData::Imp
{
public:
	vector<FEBioAppModel*>	m_modelList;

	Imp()
	{ 
		for (int i = 0; i < m_modelList.size(); ++i) delete m_modelList[i];
		m_modelList.clear();
	}

	~Imp() { }
};

//-----------------------------------------------------------------------------
FEBioData::FEBioData() : im(*(new FEBioData::Imp))
{

}

FEBioData::~FEBioData()
{
	delete &im;
}

bool FEBioData::AddModel(const std::string& modelId, const std::string& fileName, const char* sztask)
{
	FEBioAppModel* m = new FEBioAppModel(this);
	m->m_id = (int)im.m_modelList.size();
	im.m_modelList.push_back(m);

	m->m_fem.SetName(modelId);
	m->m_fileName = fileName;

	m->m_fem.AddCallback(febio_cb, CB_ALWAYS, m);

	if (sztask)
	{
		char sz[256] = { 0 };
		strcpy(sz, sztask);
		char* c = strchr(sz, ':');
		if (c)
		{
			*c++ = 0;
			m->m_taskFile = c;
		}

		m->m_task = fecore_new<FECoreTask>(sz, &m->m_fem);
	}

	return m->m_fem.Input(fileName.c_str());
}

int FEBioData::Models()
{
	return (int)im.m_modelList.size();
}

int FEBioData::GetModelIndex(const std::string& modelId)
{
	for (int i = 0; i < Models(); ++i)
	{
		if (im.m_modelList[i]->m_fem.GetName() == modelId) return i;
	}
	return -1;
}

bool FEBioData::InitFEBio()
{
	// initialize the FEBio library
	febio::InitLibrary();

#ifdef _DEBUG
	printf("FEBio version %d.%d.%d\n\n", VERSION, SUBVERSION, SUBSUBVERSION);
#endif

	char szpath[1024] = { 0 };
	if (febio::get_app_path(szpath, 1023) == 0)
	{
		char sz[1024] = { 0 };

		char* ch = strrchr(szpath, '\\');
		if (ch == 0) ch = strrchr(szpath, '/');
		if (ch) ch[1] = 0;

		sprintf(sz, "%sfebio.xml", szpath);

		FEBioConfig config;
		bool b = febio::Configure(sz, config);

		if (b == false)
		{
			printf("ERROR: Failed to read configuration file.\n");
		}

		return b;
	}

	return false;

}

bool FEBioData::HasTask(int index) const
{
	return (im.m_modelList[index]->m_task != nullptr);
}

bool FEBioData::InitModel(int index)
{
	im.m_modelList[index]->m_fem.SetLogLevel(0);
	bool b = im.m_modelList[index]->InitModel();

	im.m_modelList[index]->m_modelInitialized = b;

	return b;
}

bool FEBioData::ResetModel(int index)
{
	return im.m_modelList[index]->m_fem.Reset();
}

bool FEBioData::SolveModel(int index)
{
	im.m_modelList[index]->m_runStatus = FEBioData::RUNNING;
	bool bret = im.m_modelList[index]->Solve();
	im.m_modelList[index]->m_runStatus = FEBioData::STOPPED;
	return bret;
}

void FEBioData::FEBioCallback(int modelIndex, unsigned int nwhen)
{
	switch (nwhen)
	{
	case CB_INIT       : emit modelInit   (modelIndex); break;
	case CB_MAJOR_ITERS: emit timeStepDone(modelIndex); break;
	case CB_RESET      : emit modelReset  (modelIndex); break;
	}
}

FEBioParam FEBioData::GetFEBioParameter(const std::string& paramName)
{
	ParamString ps(paramName.c_str());

	int N = Models();
	for (int i = 0; i < N; ++i)
	{
		string modelName = im.m_modelList[i]->m_fem.GetName();
		if (ps == modelName)
		{
			FEBioParam param;
			param.SetParameter(paramName, &im.m_modelList[i]->m_fem);
			return param;
		}
	}

	return FEBioParam();
}


std::vector<FEBioParam>	FEBioData::GetFEBioParameterList(const std::string& name)
{
	std::vector<FEBioParam> paramList;

	ParamString ps(name.c_str());

	int N = Models();
	for (int i = 0; i < N; ++i)
	{
		string modelName = im.m_modelList[i]->m_fem.GetName();
		if (ps == modelName)
		{
			FECoreBase* pc = im.m_modelList[i]->m_fem.FindComponent(ps);

			FEParameterList& pl = pc->GetParameterList();
			int n = pl.Parameters();
			list<FEParam>::iterator it = pl.first();
			for (int i = 0; i < n; ++i, ++it)
			{
				FEParam& pi = *it;

				if (pi.dim() == 1)
				{
					FEParamValue val = pi.paramValue();

					switch (val.type())
					{
					case FE_PARAM_DOUBLE:
					case FE_PARAM_INT:
					case FE_PARAM_BOOL:
					{
						FEBioParam p;
						p.SetParameter(pi.name(), val);
						paramList.push_back(p);
					}
					break;
					}
				}
			}

			return paramList;
		}
	}

	return paramList;
}

FEBioApp::GLMesh* FEBioData::BuildGLMesh(int modelIndex)
{
	FEModel& fem = im.m_modelList[modelIndex]->m_fem;

	FEMesh& febMesh = fem.GetMesh();
	FESurface* surf = febMesh.ElementBoundarySurface();

	// Create the GL mesh
	int NE = surf->Elements();

	int NF = 0;
	for (int i = 0; i<NE; ++i)
	{
		FESurfaceElement& el = surf->Element(i);
		if (el.Nodes() == 3) NF++;
		else if (el.Nodes() == 4) NF += 2;
	}

	FEBioApp::GLMesh* mesh = new FEBioApp::GLMesh;
	mesh->SetModelId(modelIndex);
	mesh->Create(NF);

	// create the connectivity
	NF = 0;
	for (int i = 0; i<NE; ++i)
	{
		FESurfaceElement& el = surf->Element(i);
		FEBioApp::GLMesh::FACE& f1 = mesh->Face(NF++);
		f1.nid[0] = el.m_node[0];
		f1.nid[1] = el.m_node[1];
		f1.nid[2] = el.m_node[2];

		if (el.Nodes() == 4)
		{
			FEBioApp::GLMesh::FACE& f2 = mesh->Face(NF++);
			f2.nid[0] = el.m_node[2];
			f2.nid[1] = el.m_node[3];
			f2.nid[2] = el.m_node[0];
		}
	}

	// copy initial nodal coordinates
	for (int i = 0; i<NF; ++i)
	{
		FEBioApp::GLMesh::FACE& f = mesh->Face(i);
		vec3d& r0 = febMesh.Node(f.nid[0]).m_rt; mesh->SetVertexPosition(3*i  , FEBioApp::GLMesh::POINT(r0.x, r0.y, r0.z));
		vec3d& r1 = febMesh.Node(f.nid[1]).m_rt; mesh->SetVertexPosition(3*i+1, FEBioApp::GLMesh::POINT(r1.x, r1.y, r1.z));
		vec3d& r2 = febMesh.Node(f.nid[2]).m_rt; mesh->SetVertexPosition(3*i+2, FEBioApp::GLMesh::POINT(r2.x, r2.y, r2.z));
	}

	mesh->Update();

	return mesh;
}

void FEBioData::UpdateGLMesh(FEBioApp::GLMesh* mesh, const std::string& map)
{
	int modelId = mesh->GetModelId();

	FEMesh& febioMesh = im.m_modelList[modelId]->m_fem.GetMesh();

	// copy nodal coordinates
	int NF = mesh->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEBioApp::GLMesh::FACE& f = mesh->Face(i);
		vec3d& r0 = febioMesh.Node(f.nid[0]).m_rt; mesh->SetVertexPosition(3*i  , FEBioApp::GLMesh::POINT(r0.x, r0.y, r0.z));
		vec3d& r1 = febioMesh.Node(f.nid[1]).m_rt; mesh->SetVertexPosition(3*i+1, FEBioApp::GLMesh::POINT(r1.x, r1.y, r1.z));
		vec3d& r2 = febioMesh.Node(f.nid[2]).m_rt; mesh->SetVertexPosition(3*i+2, FEBioApp::GLMesh::POINT(r2.x, r2.y, r2.z));
	}

	int NN = mesh->Vertices();

	if (map.empty())
	{
		for (int i = 0; i < NN; ++i) mesh->SetVertexTexCoord1D(i, 0.0);
	}
	else
	{
		FEModel& fem = im.m_modelList[modelId]->m_fem;
		DOFS& dofs = fem.GetDOFS();
		int nvar = dofs.GetVariableIndex(map.c_str());
		if (nvar >= 0)
		{
			int ndof = dofs.GetVariableSize(nvar);
			int dof0 = dofs.GetDOF(nvar, 0);

			// evaluate data range
			double Dmin = 1e99, Dmax = -1e99;
			for (int i = 0; i<febioMesh.Nodes(); ++i)
			{
				FENode& ni = febioMesh.Node(i);

				double D = 0;
				if (ndof == 1)
				{
					D = ni.get(dof0);
				}
				else
				{
					for (int j = 0; j<ndof; ++j)
					{
						double dn = ni.get(dof0 + j);
						D += dn*dn;
					}
				}

				if (D < Dmin) Dmin = D;
				else if (D > Dmax) Dmax = D;
			}

			if (Dmax == Dmin) Dmax++;

			// evaluate surface values
			for (int i = 0; i<NF; ++i)
			{
				FEBioApp::GLMesh::FACE& fi = mesh->Face(i);
				for (int j = 0; j < 3; ++j)
				{
					FENode& ni = febioMesh.Node(fi.nid[j]);

					double D = 0;
					if (ndof == 1)
					{
						D = ni.get(dof0);
					}
					else
					{
						for (int j = 0; j < ndof; ++j)
						{
							double dn = ni.get(dof0 + j);
							D += dn*dn;
						}
					}

					mesh->SetVertexTexCoord1D(3 * i + j, D);
				}
			}

			im.m_modelList[modelId]->m_dataRange[0] = Dmin;
			im.m_modelList[modelId]->m_dataRange[1] = Dmax;

			// normalize texture coordinates
			for (int i = 0; i < NN; ++i)
			{
				double v = mesh->GetVertexTexCoord1D(i);
				v = (v - Dmin) / (Dmax - Dmin);
				mesh->SetVertexTexCoord1D(i, v);
			}
		}
	}

	// recalculate normals
	mesh->UpdateNormals();
}

double FEBioData::GetSimulationTime(int index) const
{
	return im.m_modelList[index]->m_fem.GetTime().currentTime;
}

int FEBioData::GetFEBioStatus(int index) const
{
	return im.m_modelList[index]->m_runStatus;
}

void FEBioData::GetDataRange(int index, double rng[2])
{
	rng[0] = im.m_modelList[index]->m_dataRange[0];
	rng[1] = im.m_modelList[index]->m_dataRange[1];
}

void FEBioData::SetModelId(int index, const std::string& id)
{
	im.m_modelList[index]->m_fem.SetName(id);
}

std::string FEBioData::GetModelId(int i)
{
	return im.m_modelList[i]->m_fem.GetName();
}

bool FEBioData::IsModelInitialized(int i)
{
	return im.m_modelList[i]->m_modelInitialized;
}

std::string FEBioData::GetModelFile(int i)
{
	return im.m_modelList[i]->m_fileName;
}

void FEBioData::StopModel(int index)
{
	im.m_modelList[index]->m_bforceStop = true;
	im.m_modelList[index]->m_runStatus = FEBioData::STOPPED;
}

void FEBioData::StopAll()
{
	for (int i = 0; i < Models(); ++i) StopModel(i);
}

bool FEBioData::ForceStop(int index)
{
	return im.m_modelList[index]->m_bforceStop;
}

FEMODEL_PTR FEBioData::GetFEModel()
{
	return (void*) (&im.m_modelList[0]->m_fem);
}
