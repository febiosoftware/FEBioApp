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
		int		node[3];	// node indices
		vec3d	fnorm;		// face normal
		vec3d	norm[3];	// node normals
		int		nbr[3];		// indices to neighbors (or -1 if no neighbor)
	};

	struct NODE
	{
		vec3d	pos;	//!< node position
	};

public:
	GLMesh();

	void Clear();

	void Create(int nodes, int faces);

	void Render();

	void UpdateFaces();

	void UpdateNormals();

	void PartitionFaces(double wAngle = 60.0);

public:
	int Nodes() { return (int) m_Node.size(); }
	int Faces() { return (int) m_Face.size(); }

	NODE& Node(int i) { return m_Node[i]; }
	FACE& Face(int i) { return m_Face[i]; }

private:
	vector<NODE>	m_Node;
	vector<FACE>	m_Face;
};
