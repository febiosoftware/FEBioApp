#include "stdafx.h"
#include "GLMesh.h"
#include <Windows.h>
#include <gl/GL.h>
#include <stack>

GLMesh::GLMesh()
{
}

void GLMesh::Clear()
{
	m_Node.clear();
	m_Face.clear();
}

void GLMesh::Create(int nodes, int faces)
{
	if (nodes > 0) m_Node.resize(nodes);
	if (faces > 0) m_Face.resize(faces);
}

void GLMesh::Render()
{
	int NF = Faces();
	if (NF > 0)
	{
		glBegin(GL_TRIANGLES);
		{
			for (int i=0; i<NF; ++i)
			{
				FACE& f = Face(i);

				vec3d& n0 = f.norm[0];
				vec3d& n1 = f.norm[1];
				vec3d& n2 = f.norm[2];
			
				vec3d& r0 = Node(f.node[0]).pos;
				vec3d& r1 = Node(f.node[1]).pos;
				vec3d& r2 = Node(f.node[2]).pos;

				glNormal3d(n0.x, n0.y, n0.z); glVertex3d(r0.x, r0.y, r0.z);
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
			}
		}
		glEnd();
	}
}

void GLMesh::UpdateFaces()
{
	const int NN = Nodes();
	const int NF = Faces();

	// clear all neighbors
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);
		f.nbr[0] = f.nbr[1] = f.nbr[2] = -1;
	}

	// build the node-face list
	vector< vector<int> > NFL; NFL.resize(NN);
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);
		NFL[f.node[0]].push_back(i);
		NFL[f.node[1]].push_back(i);
		NFL[f.node[2]].push_back(i);
	}

	// find all neighbors
	for (int i=0; i<NF; ++i)
	{
		FACE& fi = Face(i);
		int* ni = fi.node;

		for (int k=0; k<3; ++k)
		{
			if (fi.nbr[k] == -1)
			{
				int n0 = ni[k];
				int n1 = ni[(k+1)%3];
				vector<int>& FL = NFL[n0];
				int nval = FL.size();
				for (int j=0; j<nval; ++j)
				{
					if (FL[j] != i)
					{
						FACE& fj = Face(FL[j]);
						int* nj = fj.node;
						for (int l=0; l<3; ++l)
						{
							int m0 = nj[l];
							int m1 = nj[(l+1)%3];

							if (((n0==m0)&&(n1==m1))||
								((n0==m1)&&(n1==m0)))
							{
								fi.nbr[k] = FL[j];
								fj.nbr[l] = i;
								break;
							}
						}
					}
					if (fi.nbr[k] != -1) break;
				}
			}
		}
	}	
}

void GLMesh::PartitionFaces(double wAngle)
{
	const double tol = cos(wAngle*3.1415926/180.0);

	const int NF = Faces();

	// clear the paritions
	for (int i=0; i<NF; ++i) Face(i).pid = -1;

	// we need to update the face normals
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);

		vec3d& r0 = Node(f.node[0]).pos;
		vec3d& r1 = Node(f.node[1]).pos;
		vec3d& r2 = Node(f.node[2]).pos;

		vec3d faceNorm = (r1 - r0) ^ (r2 - r0);
		faceNorm.unit();

		f.fnorm = faceNorm;
	}

	// assign partitions
	int ng = 0;
	int n = 0;
	int faceIndex = 0;
	while (faceIndex >= 0)
	{
		FACE* pf = &Face(faceIndex);
		pf->pid = ng;
		stack<FACE*> faceStack;
		faceStack.push(pf);
		while (faceStack.empty() == false)
		{
			pf = faceStack.top(); faceStack.pop();

			// push neighbors
			for (int i=0; i<3; ++i)
			{
				if (pf->nbr[i] >= 0)
				{
					FACE* pf2 = &Face(pf->nbr[i]);
					if (pf2->pid == -1)
					{
						double w = pf->fnorm*pf2->fnorm;
						if (w > tol)
						{
							pf2->pid = ng;
							faceStack.push(pf2);
						}
					}
				}
			}
		}

		// increase partition counter
		ng++;

		// find an unassigned face
		faceIndex = -1;
		for (; n<NF; ++n)
		{
			if (Face(n).pid == -1)
			{
				faceIndex = n++;
				break;
			}
		}
	}
}

void GLMesh::UpdateNormals()
{
	const int NN = Nodes();
	const int NF = Faces();

	// update face normals first
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);

		vec3d& r0 = Node(f.node[0]).pos;
		vec3d& r1 = Node(f.node[1]).pos;
		vec3d& r2 = Node(f.node[2]).pos;

		vec3d faceNorm = (r1 - r0) ^ (r2 - r0);
		faceNorm.unit();

		f.fnorm = faceNorm;
	}

	// clear all node normals
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);
		f.norm[0] = f.norm[1] = f.norm[2] = vec3d(0,0,0);
	}

	// assign node normals
	stack<FACE*> faceStack;
	vector<int> tag(NF);
	int ng = 0;
	int faceIndex = 0;
	vector<vec3d> norm;
	while (faceIndex < NF)
	{
		norm.assign(NN, vec3d(0,0,0));
		tag.assign(NF, 0);
		FACE* pf = &Face(faceIndex);
		tag[faceIndex] = 1;
		faceStack.push(pf);
		while (faceStack.empty() == false)
		{
			pf = faceStack.top(); faceStack.pop();
			vec3d& ni = pf->fnorm;

			norm[pf->node[0]] += ni;
			norm[pf->node[1]] += ni;
			norm[pf->node[2]] += ni;

			// push neighbors
			for (int i=0; i<3; ++i)
			{
				if (pf->nbr[i] >= 0)
				{
					FACE* pf2 = &Face(pf->nbr[i]);
					if ((pf2->pid == pf->pid) && (tag[pf->nbr[i]] == 0))
					{
						tag[pf->nbr[i]] = 1;
						faceStack.push(pf2);
					}
				}
			}
		}

		// assign normals
		for (int i=0; i<NF; ++i)
		{
			if (tag[i] == 1)
			{
				FACE& fi = Face(i);
				fi.norm[0] = norm[fi.node[0]];
				fi.norm[1] = norm[fi.node[1]];
				fi.norm[2] = norm[fi.node[2]];
			}
		}

		// find an unassigned face
		for (; faceIndex < NF; ++faceIndex)
		{
			if (Face(faceIndex).pid == ng+1) { ng++; break; }
		}
	}

	// normalize node normals
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);
		f.norm[0].unit();
		f.norm[1].unit();
		f.norm[2].unit();
	}
}
