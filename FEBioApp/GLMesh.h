#pragma once
#include <FECore/vec3d.h>
#include <vector>
using namespace std;

class GLMesh
{
public:
	struct FACE
	{
		int		pid;		// parition number (used for smoothing)
		vec3d	fnorm;		// face normal
		int		nbr[3];		// indices to neighbors (or -1 if no neighbor)
		int		nid[3];		// node IDs
		int		lnode[3];	// (local) node indices (into m_Node, m_Norm arrays)
	};

public:
	GLMesh();

	void Clear();

	void Create(int faces);

	void Render();

public:

	// find the face neighbors
	void UpdateFaces();

	// partition the faces
	void PartitionFaces(double wAngle = 60.0);

	// update face normals
	void UpdateNormals();

public:
	int Faces() { return (int) m_Face.size(); }

	FACE& Face(int i) { return m_Face[i]; }

	vec3d& nodePosition(int i) { return m_Node[i]; }
	vec3d& nodeNormal(int i) { return m_Norm[i]; }
	double& nodeTexCoord1D(int i) { return m_Tex[i]; }

private:
	vector<FACE>	m_Face;
	vector<vec3d>	m_Node;	// node positions
	vector<vec3d>	m_Norm;	// node normals
	vector<double>	m_Tex;	// node texture coordinates
};
