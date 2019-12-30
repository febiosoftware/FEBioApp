#pragma once
#include "febioapp_api.h"
#include <vector>
using namespace std;

namespace FEBioApp {

class FEBIO_APP_API GLMesh
{
public:
	struct POINT
	{
		double	x, y, z;

		POINT()
		{
			x = y = z = 0;
		}

		POINT(double X, double Y, double Z)
		{
			x = X;
			y = Y;
			z = Z;
		}

		void operator += (const POINT& p)
		{
			x += p.x;
			y += p.y;
			z += p.z;
		}
	};

public:
	struct FACE
	{
		int		pid;		// parition number (used for smoothing)
		int		nbr[3];		// indices to neighbors (or -1 if no neighbor)

		int		nid[3];		// node IDs into FEBio mesh
		int		lid[3];		// local IDs into Node array
		POINT	faceNormal;
	};

public:
	GLMesh();

	void Clear();

	void Create(int faces);

	void Render();

	void SetModelId(int id);
	int GetModelId() const;

public:
	void Update();

	// find the face neighbors
	void UpdateFaces();

	// partition the faces
	void PartitionFaces(double wAngle = 60.0);

	// update face normals
	void UpdateNormals();

	void SetVertexPosition(int node, const POINT& p)
	{
		m_Vert[3 * node  ] = p.x;
		m_Vert[3 * node+1] = p.y;
		m_Vert[3 * node+2] = p.z;
	}

	POINT GetVertexPosition(int node) const
	{
		POINT p = { m_Vert[3 * node], m_Vert[3 * node + 1], m_Vert[3 * node + 2] };
		return p;
	}

	void SetVertexNormal(int node, const POINT& n)
	{
		m_Norm[3*node  ] = n.x;
		m_Norm[3*node+1] = n.y;
		m_Norm[3*node+2] = n.z;
	}

	POINT GetVertexNormal(int node) const
	{
		POINT n = { m_Norm[3 * node], m_Norm[3 * node + 1], m_Norm[3 * node + 2] };
		return n;
	}

	void SetVertexTexCoord1D(int node, double v) { m_Tex[node] = v; }

	double GetVertexTexCoord1D(int node) { return m_Tex[node]; }

public:
	int Faces() const { return (int) m_Face.size(); }

	int Vertices() const { return 3 * Faces(); }

	FACE& Face(int i) { return m_Face[i]; }
	const FACE& Face(int i) const { return m_Face[i]; }

private:
	void BuildNodeList();

private:
	int				m_modelId;
	vector<FACE>	m_Face;

	vector<double>	m_Vert;
	vector<double>	m_Norm;
	vector<double>	m_Tex;

	vector<int>		m_Node;	// unique node list
};

inline double dot(const GLMesh::POINT& a, const GLMesh::POINT& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

FEBIO_APP_API double MeshSize(const GLMesh& mesh);

FEBIO_APP_API GLMesh::POINT MeshCenter(const GLMesh& mesh);

}
