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

bool febio_cb(FEModel* pfem, unsigned int nwhen, void* pd)
{
	FEBioData* pThis = (FEBioData*)pd;
	return pThis->FEBioCallback(nwhen);
}

//-----------------------------------------------------------------------------
class FEBioData::Imp
{
public:
	FEBioModel		m_fem;
	FECoreTask*		m_task;
	std::string 	m_taskFile;

	double		m_dataRange[2];

	FEBioData::FEBIO_STATUS	m_runStatus;

	Imp() : m_task(0) 
	{ 
		m_runStatus = FEBioData::STOPPED; 
		m_dataRange[0] = 0.0;
		m_dataRange[1] = 1.0;
	}

	~Imp() { if (m_task) delete m_task; m_task = 0; }
};

//-----------------------------------------------------------------------------
FEBioData::FEBioData() : im(new FEBioData::Imp)
{
	im->m_fem.AddCallback(febio_cb, CB_ALWAYS, this);
}

FEBioData::~FEBioData()
{
	delete im;
	im = nullptr;
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

		bool b = febio::Configure(sz);

		if (b == false)
		{
			printf("ERROR: Failed to read configuration file.\n");
		}
	}

	return false;

}

bool FEBioData::HasTask() const
{
	return (im->m_task != nullptr);
}

bool FEBioData::InitModel()
{
	im->m_fem.SetLogLevel(0);
	if (im->m_fem.Init() == false) return false;

/*	if (im->m_task == nullptr)
	{
		im->m_task = fecore_new<FECoreTask>("solve", &im->m_fem);
		if (im->m_task == nullptr) return false;
	}

	return im->m_task->Init(im->m_taskFile.c_str());
*/
	return true;
}

bool FEBioData::ResetModel()
{
	return im->m_fem.Reset();
}

bool FEBioData::RunModel()
{
	return im->m_task->Run();
}

bool FEBioData::SolveModel()
{
	im->m_runStatus = FEBioData::RUNNING;
	bool bret = im->m_fem.Solve();
	im->m_runStatus = FEBioData::STOPPED;
	return bret;
}

bool FEBioData::ReadFEBioFile(const char* szfile)
{
	return im->m_fem.Input(szfile);
}

bool FEBioData::SetFEBioTask(const char* sztaskName, const char* sztaskFile)
{
	im->m_task = fecore_new<FECoreTask>(FETASK_ID, sztaskName, &im->m_fem);
	if (im->m_task == 0) return false;
	im->m_taskFile = sztaskFile;
	return true;
}

bool FEBioData::FEBioCallback(unsigned int nwhen)
{
	if (im->m_runStatus == FEBioData::STOPPED) return false;

	switch (nwhen)
	{
	case CB_INIT       : emit modelInit   (); break;
	case CB_MAJOR_ITERS: emit timeStepDone(); break;
	}

	return true;
}

FEBioParam FEBioData::GetFEBioParameter(const std::string& paramName)
{
	ParamString ps(paramName.c_str());
	FEParamValue val = im->m_fem.GetParameterValue(ps);
	FEBioParam param;
	param.SetParameter(paramName, val);
	return param;
}


std::vector<FEBioParam>	FEBioData::GetFEBioParameterList(const std::string& name)
{
	ParamString ps(name.c_str());
	FECoreBase* pc = im->m_fem.FindComponent(ps);

	std::vector<FEBioParam> paramList;

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

FEBioApp::GLMesh* FEBioData::BuildGLMesh()
{
	FEModel& fem = im->m_fem;

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
		vec3d& r0 = febMesh.Node(f.nid[0]).m_rt; mesh->SetNodePosition(3*i  , FEBioApp::GLMesh::POINT(r0.x, r0.y, r0.z));
		vec3d& r1 = febMesh.Node(f.nid[1]).m_rt; mesh->SetNodePosition(3*i+1, FEBioApp::GLMesh::POINT(r1.x, r1.y, r1.z));
		vec3d& r2 = febMesh.Node(f.nid[2]).m_rt; mesh->SetNodePosition(3*i+2, FEBioApp::GLMesh::POINT(r2.x, r2.y, r2.z));
	}

	// find the face neighbors
	mesh->UpdateFaces();

	mesh->PartitionFaces();

	mesh->UpdateNormals();

	return mesh;
}

void FEBioData::UpdateGLMesh(FEBioApp::GLMesh* mesh, const std::string& map)
{
	FEMesh& febioMesh = im->m_fem.GetMesh();

	// copy nodal coordinates
	int NF = mesh->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEBioApp::GLMesh::FACE& f = mesh->Face(i);
		vec3d& r0 = febioMesh.Node(f.nid[0]).m_rt; mesh->SetNodePosition(3*i  , FEBioApp::GLMesh::POINT(r0.x, r0.y, r0.z));
		vec3d& r1 = febioMesh.Node(f.nid[1]).m_rt; mesh->SetNodePosition(3*i+1, FEBioApp::GLMesh::POINT(r1.x, r1.y, r1.z));
		vec3d& r2 = febioMesh.Node(f.nid[2]).m_rt; mesh->SetNodePosition(3*i+2, FEBioApp::GLMesh::POINT(r2.x, r2.y, r2.z));
	}

	int NN = mesh->Nodes();

	if (map.empty())
	{
		for (int i = 0; i < NN; ++i) mesh->SetNodeTexCoord1D(i, 0.0);
	}
	else
	{
		FEModel& fem = im->m_fem;
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

					mesh->SetNodeTexCoord1D(3 * i + j, D);
				}
			}

			im->m_dataRange[0] = Dmin;
			im->m_dataRange[1] = Dmax;

			// normalize texture coordinates
			for (int i = 0; i < NN; ++i)
			{
				double v = mesh->GetNodeTexCoord1D(i);
				v = (v - Dmin) / (Dmax - Dmin);
				mesh->SetNodeTexCoord1D(i, v);
			}
		}
	}

	// recalculate normals
	mesh->UpdateNormals();
}

double FEBioData::GetSimulationTime() const
{
	return im->m_fem.GetTime().currentTime;
}

int FEBioData::GetFEBioStatus() const
{
	return im->m_runStatus;
}

void FEBioData::SetFEBioStatus(FEBIO_STATUS s)
{
	im->m_runStatus = s;
}

void FEBioData::GetDataRange(double rng[2])
{
	rng[0] = im->m_dataRange[0];
	rng[1] = im->m_dataRange[1];
}
