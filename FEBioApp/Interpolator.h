#pragma once
#include <FECore/mat3d.h>
#include <FECore/quatd.h>

//=============================================================================
//! This class implements an interpolator that interpolates between two values.
class Interpolator 
{
public:
	Interpolator();

	bool Update();

	void Target(double g);

	double Value() const { return m_gt; }

	void HitTarget();

	double Target() const { return m_g1; }

public:
	double	m_g0;	// starting value
	double	m_g1;	// target value
	double	m_gt;	// current value
	double	m_at;	// interpolation value
	double	m_da;	// update value
	bool	m_banim;

public:
	static int		m_nsteps;
	static double	m_smooth;	// 0 = linear, 1 < smooth
};

//=============================================================================
//! special interpolator for vectors
class VecInterpolator
{
public:
	VecInterpolator();

	bool Update();

	void Target(const vec3d& q);

	vec3d Value() const { return m_vt; }

	void HitTarget();

	vec3d Target() const { return m_v1; }

public:
	vec3d	m_v0;	// starting value
	vec3d	m_v1;	// target value
	vec3d	m_vt;	// current value
	double	m_at;	// interpolation value (between 0 and 1)
	double	m_da;	// update value
	bool	m_banim;
};


//=============================================================================
//! special interpolator for quaternions
class QuatInterpolator
{
public:
	QuatInterpolator();

	bool Update();

	void Target(const quatd& q);

	quatd Value() const { return m_qt; }

	void HitTarget();

	quatd Target() const { return m_q1; }

public:
	quatd	m_q0;	// starting value
	quatd	m_q1;	// target value
	quatd	m_qt;	// current value
	double	m_at;	// interpolation value (between 0 and 1)
	double	m_da;	// update value
	bool	m_banim;
};
