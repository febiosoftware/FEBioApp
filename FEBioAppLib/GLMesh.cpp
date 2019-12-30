//#include "stdafx.h"
#include "GLMesh.h"
#include <Windows.h>
#include <gl/GL.h>
#include <stack>
#include <assert.h>

using namespace FEBioApp;

GLMesh::GLMesh()
{
	m_modelId = -1;
}

void GLMesh::Clear()
{
	m_Face.clear();
}

void GLMesh::SetModelId(int id)
{
	m_modelId = id;
}

int GLMesh::GetModelId() const
{
	return m_modelId;
}

void GLMesh::Create(int faces)
{
	if (faces > 0)
	{
		m_Face.resize(faces);

		int nodes = 3 * faces;
		m_Vert.resize(3*nodes);
		m_Norm.resize(3*nodes);
		m_Tex.resize(nodes, 0.0);
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

		glVertexPointer(3, GL_DOUBLE, 0, &m_Vert[0]);
		glNormalPointer(GL_DOUBLE, 0, &m_Norm[0]);
		glTexCoordPointer(1, GL_DOUBLE, 0, &m_Tex[0]);

		glDrawArrays(GL_TRIANGLES, 0, 3*m_Face.size());

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

GLMesh::POINT NormalToPlane(const GLMesh::POINT& a, const GLMesh::POINT& b, const GLMesh::POINT& c)
{
	GLMesh::POINT e1 = { b.x - a.x, b.y - a.y, b.z - a.z };
	GLMesh::POINT e2 = { c.x - a.x, c.y - a.y, c.z - a.z };

	GLMesh::POINT n = { 
		e1.y*e2.z - e1.z*e2.y, 
		e1.z*e2.x - e1.x*e2.z,
		e1.x*e2.y - e1.y*e2.x
	};

	double L = sqrt(dot(n, n));
	if (L != 0.0) { n.x /= L; n.y /= L; n.z /= L; }

	return n;
}

void Normalize(GLMesh::POINT& n)
{
	double L = sqrt(dot(n, n));
	if (L != 0.0) { n.x /= L; n.y /= L; n.z /= L; }
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

		POINT r0 = GetVertexPosition(i*3  );
		POINT r1 = GetVertexPosition(i*3+1);
		POINT r2 = GetVertexPosition(i*3+2);

		f.faceNormal = NormalToPlane(r0, r1, r2);
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
						double w = dot(pf->faceNormal, pf2->faceNormal);
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
	for (int i = 0; i<NF; ++i)
	{
		FACE& f = Face(i);

		POINT r0 = GetVertexPosition(i * 3);
		POINT r1 = GetVertexPosition(i * 3 + 1);
		POINT r2 = GetVertexPosition(i * 3 + 2);

		f.faceNormal = NormalToPlane(r0, r1, r2);
	}

	// clear all node normals
	m_Norm.assign(m_Norm.size(), 0.0);

	// assign node normals
	int NN = m_Node.size();
	stack<int> faceStack;
	vector<int> tag(NF);
	int ng = 0;
	int faceIndex = 0;
	vector<POINT> norm;
	while (faceIndex < NF)
	{
		norm.assign(NN, POINT(0,0,0));
		tag.assign(NF, 0);
		FACE* pf = &Face(faceIndex);
		tag[faceIndex] = 1;
		faceStack.push(faceIndex);
		while (faceStack.empty() == false)
		{
			int fid = faceStack.top(); faceStack.pop();
			pf = &Face(fid);
			POINT& ni = pf->faceNormal;

			norm[pf->lid[0]] += ni;
			norm[pf->lid[1]] += ni;
			norm[pf->lid[2]] += ni;

			// push neighbors
			for (int i=0; i<3; ++i)
			{
				int fjd = pf->nbr[i];
				if (fjd >= 0)
				{
					FACE* pf2 = &Face(pf->nbr[i]);
					if ((pf2->pid == pf->pid) && (tag[pf->nbr[i]] == 0))
					{
						tag[pf->nbr[i]] = 1;
						faceStack.push(fjd);
					}
				}
			}
		}

		// assign normals
		for (int i=0; i<NF; ++i)
		{
			if (tag[i] == 1)
			{
				FACE& fi = m_Face[i];
				SetVertexNormal(3*i  , norm[fi.lid[0]]);
				SetVertexNormal(3*i+1, norm[fi.lid[1]]);
				SetVertexNormal(3*i+2, norm[fi.lid[2]]);
			}
		}

		// find an unassigned face
		for (; faceIndex < NF; ++faceIndex)
		{
			if (Face(faceIndex).pid == ng+1) { ng++; break; }
		}
	}

	// normalize node normals
	for (int i = 0; i < NF; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			POINT n = GetVertexNormal(3 * i + j);
			Normalize(n);
			SetVertexNormal(3 * i + j, n);
		}
	}
}

void GLMesh::Update()
{
	BuildNodeList();
	UpdateFaces();
	PartitionFaces();
	UpdateNormals();
}

void GLMesh::BuildNodeList()
{
	m_Node.clear();

	int minid = -1, maxid = -1;
	for (int i = 0; i < Faces(); ++i)
	{
		FACE& fi = m_Face[i];

		for (int j = 0; j < 3; ++j)
		{
			int nj = fi.nid[j];

			if (minid == -1) minid = nj;
			else if (nj < minid) minid = nj;

			if (nj > maxid) maxid = nj;
		}
	}

	int nsize = maxid - minid + 1;
	vector<int> tmp(nsize, -1);
	for (int i = 0; i < Faces(); ++i)
	{
		FACE& fi = m_Face[i];

		for (int j = 0; j < 3; ++j)
		{
			int nj = fi.nid[j];

			tmp[nj - minid] = 1;
		}
	}

	int n = 0;
	for (int i = 0; i < nsize; ++i)
	{
		if (tmp[i] != -1)
		{
			tmp[i] = n++;
			m_Node.push_back(minid + i);
		}
	}

	for (int i = 0; i < Faces(); ++i)
	{
		FACE& fi = m_Face[i];

		for (int j = 0; j < 3; ++j)
		{
			int nj = fi.nid[j];
			fi.lid[j] = tmp[nj - minid];
			assert(fi.lid[j] != -1);
			assert(m_Node[fi.lid[j]] == nj);
		}
	}
}

double FEBioApp::MeshSize(const GLMesh& mesh)
{
	int NN = mesh.Vertices();
	if (NN == 0) return 0.0;

	GLMesh::POINT p0, p1;
	p0 = p1 = mesh.GetVertexPosition(0);

	for (int i = 0; i < 3*mesh.Faces(); ++i)
	{
		GLMesh::POINT p = mesh.GetVertexPosition(i);

		if (p.x < p0.x) p0.x = p.x;
		if (p.y < p0.y) p0.y = p.y;
		if (p.z < p0.z) p0.z = p.z;

		if (p.x > p1.x) p1.x = p.x;
		if (p.y > p1.y) p1.y = p.y;
		if (p.z > p1.z) p1.z = p.z;
	}

	double dx = p1.x - p0.x;
	double dy = p1.y - p0.y;
	double dz = p1.z - p0.z;

	double r = (dx > dy ? dx : dy);
	r = (r > dz ? r : dz);

	return r;
}

FEBioApp::GLMesh::POINT FEBioApp::MeshCenter(const GLMesh& mesh)
{
	int NN = mesh.Vertices();
	if (NN == 0) return GLMesh::POINT(0,0,0);

	GLMesh::POINT p0, p1;
	p0 = p1 = mesh.GetVertexPosition(0);

	for (int i = 0; i < 3 * mesh.Faces(); ++i)
	{
		GLMesh::POINT p = mesh.GetVertexPosition(i);

		if (p.x < p0.x) p0.x = p.x;
		if (p.y < p0.y) p0.y = p.y;
		if (p.z < p0.z) p0.z = p.z;

		if (p.x > p1.x) p1.x = p.x;
		if (p.y > p1.y) p1.y = p.y;
		if (p.z > p1.z) p1.z = p.z;
	}

	return GLMesh::POINT(0.5*(p0.x + p1.x), 0.5*(p0.y + p1.y), 0.5*(p0.z + p1.z));
}
