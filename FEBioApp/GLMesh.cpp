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
	m_Face.clear();
}

void GLMesh::Create(int faces)
{
	if (faces > 0) 
	{
		int nodes = faces*3;
		m_Node.resize(nodes);
		m_Norm.resize(nodes);
		
		m_Tex.resize(nodes);
		for (size_t i=0; i<m_Tex.size(); ++i) m_Tex[i] = 0.0;

		m_Face.resize(faces);
		for (int i=0; i<faces; ++i)
		{
			FACE& f = m_Face[i];
			f.lnode[0] = 3*i  ;
			f.lnode[1] = 3*i+1;
			f.lnode[2] = 3*i+2;
		}
	}
}

void GLMesh::Render()
{
	int NF = Faces();
	if (NF > 0)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_DOUBLE, 0, &m_Node[0]);
		glNormalPointer(GL_DOUBLE   , 0, &m_Norm[0]);
		glTexCoordPointer(1, GL_DOUBLE, 0, &m_Tex[0]);

		glDrawArrays(GL_TRIANGLES, 0, m_Node.size());

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

void GLMesh::UpdateFaces()
{
	const int NF = Faces();

	// clear all neighbors
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);
		f.nbr[0] = f.nbr[1] = f.nbr[2] = -1;
	}

	// let's find out how many nodes we have
	int NN = 0;
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);
		if (f.nid[0] > NN) NN++;
		if (f.nid[1] > NN) NN++;
		if (f.nid[2] > NN) NN++;
	}
	NN++;

	// build the node-face list
	vector< vector<int> > NFL; NFL.resize(NN);
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);
		NFL[f.nid[0]].push_back(i);
		NFL[f.nid[1]].push_back(i);
		NFL[f.nid[2]].push_back(i);
	}

	// find all neighbors
	for (int i=0; i<NF; ++i)
	{
		FACE& fi = Face(i);

		for (int k=0; k<3; ++k)
		{
			if (fi.nbr[k] == -1)
			{
				int n0 = fi.nid[k      ];
				int n1 = fi.nid[(k+1)%3];

				vector<int>& FL = NFL[n0];
				int nval = FL.size();
				for (int j=0; j<nval; ++j)
				{
					if (FL[j] != i)
					{
						FACE& fj = Face(FL[j]);
						for (int l=0; l<3; ++l)
						{
							int m0 = fj.nid[l      ];
							int m1 = fj.nid[(l+1)%3];

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

		vec3d& r0 = m_Node[f.lnode[0]];
		vec3d& r1 = m_Node[f.lnode[1]];
		vec3d& r2 = m_Node[f.lnode[2]];

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
	const int NF = Faces();

	// update face normals first
	for (int i=0; i<NF; ++i)
	{
		FACE& f = Face(i);

		vec3d& r0 = m_Node[f.lnode[0]];
		vec3d& r1 = m_Node[f.lnode[1]];
		vec3d& r2 = m_Node[f.lnode[2]];

		vec3d faceNorm = (r1 - r0) ^ (r2 - r0);
		faceNorm.unit();

		f.fnorm = faceNorm;
	}

	// clear all node normals
	const int NN = m_Norm.size();
	for (int i=0; i<NN; ++i) m_Norm[i] = vec3d(0,0,0);

	// assign node normals
	stack<FACE*> faceStack;
	vector<int> tag(NF);
	int ng = 0;
	int faceIndex = 0;
	vector<vec3d> norm;
	while (faceIndex < NF)
	{
		norm.assign(3*NF, vec3d(0,0,0));
		tag.assign(NF, 0);
		FACE* pf = &Face(faceIndex);
		tag[faceIndex] = 1;
		faceStack.push(pf);
		while (faceStack.empty() == false)
		{
			pf = faceStack.top(); faceStack.pop();
			vec3d& ni = pf->fnorm;

			norm[pf->nid[0]] += ni;
			norm[pf->nid[1]] += ni;
			norm[pf->nid[2]] += ni;

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
				m_Norm[fi.lnode[0]] = norm[fi.nid[0]];
				m_Norm[fi.lnode[1]] = norm[fi.nid[1]];
				m_Norm[fi.lnode[2]] = norm[fi.nid[2]];
			}
		}

		// find an unassigned face
		for (; faceIndex < NF; ++faceIndex)
		{
			if (Face(faceIndex).pid == ng+1) { ng++; break; }
		}
	}

	// normalize node normals
	for (int i=0; i<NN; ++i) m_Norm[i].unit();
}
