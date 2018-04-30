// GLCamera.h: interface for the CGLCamera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLCAMERA_H__31B2A627_DB97_4CDD_B655_7FA0180C4A89__INCLUDED_)
#define AFX_GLCAMERA_H__31B2A627_DB97_4CDD_B655_7FA0180C4A89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <PostViewLib/math3d.h>
#include "Interpolator.h"

//-----------------------------------------------------------------------------
class GLCameraTransform
{
public:
	GLCameraTransform(){}
	GLCameraTransform(const GLCameraTransform& key);
	GLCameraTransform& operator = (const GLCameraTransform& key);

public:
	vec3f	pos;	// position
	vec3f	trg;	// target
	quat4f	rot;	// rotation
};

//-----------------------------------------------------------------------------
//! Class that represents a camera.
//! It uses the interpolater class to allow animatable transistions between
//! two viewpoints.
class CGLCamera  
{
public:
	//! contructor
	CGLCamera();

	//! destructor
	virtual ~CGLCamera();

	//! reset the camera to its initial position/orientation
	void Reset();

	//! position the camera in space
	void Transform();

	// update camera position (for animations)
	void Update(bool bhit = false);

	// set line-draw or decal mode
	void LineDrawMode(bool b) { m_bdecal = b; }

public:
	void SetCameraSpeed(double f);
	double GetCameraSpeed() { return m_speed; }

	void SetCameraBias(double f);
	double GetCameraBias() { return m_bias; }

	// --- camera movements ---
public:
	// pan the camera (rotate around camera center)
	void Pan(const quat4f& q);

	// dolly the camera (move camera + target forward/backward)
	void Dolly(float f);

	// truck the camera (move camera in plane)
	void Truck(const vec3f& r);

	// orbit camera (rotate around target)
	void Orbit(const quat4f& q);

	// zoom camera (move camera toward/away from target)
	void Zoom(float f);

public:
	// sets the distance to the target
	void SetTargetDistance(double z) { vec3f r = m_trg.Target(); r.z = z; m_trg.Target(r); }

	// gets the distance to the target
	double GetTargetDistance() { return m_trg.Value().z; }

	// gets the distance to the target
	double GetFinalTargetDistance() { return m_trg.Target().z; }

	// set the target + distance
	void SetTarget(const vec3f& r);

	// set the orientation of the camera
	void SetOrientation(const quat4f& q) { m_rot.Target(q); }

	// get the camera orientation
	quat4f GetOrientation() { return m_rot.Value(); }

	// get the target position
	vec3f GetPosition() { return m_pos.Value(); }

	vec3f GetFinalPosition() const {return m_pos.Target(); } 

	vec3f Target() const { return m_trg.Value(); }
	vec3f FinalTarget() const {return m_trg.Target(); } 

	// set the view direction
	void SetViewDirection(const vec3f& r);

	// set the camera transformation
	void SetTransform(GLCameraTransform& t);

	void GetTransform(GLCameraTransform& t);

	// see if the camera is still animating
	bool IsAnimating();

public:
	VecInterpolator		m_pos;	// position of target in global coordinates
	VecInterpolator		m_trg;	// position of target in local coordinates
	QuatInterpolator	m_rot;	// orientation of camera
	bool				m_bdecal;	//!< decal mode flag

private:
	double m_speed;
	double m_bias;
};


#endif // !defined(AFX_GLCAMERA_H__31B2A627_DB97_4CDD_B655_7FA0180C4A89__INCLUDED_)
