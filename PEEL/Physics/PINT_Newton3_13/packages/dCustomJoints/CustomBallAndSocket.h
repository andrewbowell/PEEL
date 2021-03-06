/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/


// CustomBallAndSocket.h: interface for the CustomBallAndSocket class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CUSTOMBALLANDSOCKET_H_
#define _CUSTOMBALLANDSOCKET_H_

#include "CustomJoint.h"
class CustomBallAndSocket: public CustomJoint  
{
	public:
	CUSTOM_JOINTS_API CustomBallAndSocket(const dMatrix& pinAndPivotFrame, NewtonBody* const child, NewtonBody* const parent = NULL);
	CUSTOM_JOINTS_API virtual ~CustomBallAndSocket();

	protected:
	CUSTOM_JOINTS_API virtual void GetInfo (NewtonJointRecord* const info) const;
	CUSTOM_JOINTS_API virtual void SubmitConstraints (dFloat timestep, int threadIndex);

	dMatrix m_localMatrix0;
	dMatrix m_localMatrix1;
};


// similar to the ball and socked 
// plus it has the ability to set joint limits
class CustomLimitBallAndSocket: public CustomBallAndSocket  
{
	public:
	CUSTOM_JOINTS_API CustomLimitBallAndSocket(const dMatrix& pinAndPivotFrame, NewtonBody* const child, NewtonBody* const parent = NULL);
	CUSTOM_JOINTS_API CustomLimitBallAndSocket(const dMatrix& childPinAndPivotFrame, NewtonBody* const child, const dMatrix& parentPinAndPivotFrame, NewtonBody* const parent);
	CUSTOM_JOINTS_API virtual ~CustomLimitBallAndSocket();

	CUSTOM_JOINTS_API void SetConeAngle (dFloat angle);
	CUSTOM_JOINTS_API void SetTwistAngle (dFloat minAngle, dFloat maxAngle);

	CUSTOM_JOINTS_API dFloat GetConeAngle () const;
	CUSTOM_JOINTS_API void GetTwistAngle (dFloat& minAngle, dFloat& maxAngle) const;

	protected:
	CUSTOM_JOINTS_API virtual void GetInfo (NewtonJointRecord* const info) const;
	CUSTOM_JOINTS_API virtual void SubmitConstraints (dFloat timestep, int threadIndex);


	dMatrix m_rotationOffset;
	dFloat m_coneAngle;
	dFloat m_minTwistAngle;
	dFloat m_maxTwistAngle;
	dFloat m_coneAngleCos;
	dFloat m_coneAngleSin;
	dFloat m_coneAngleHalfCos;
	dFloat m_coneAngleHalfSin;	

//	AngularIntegration m_pitch;
//	AngularIntegration m_yaw;
//	AngularIntegration m_roll;
};


class CustomControlledBallAndSocket: public CustomBallAndSocket  
{
	public:
	CUSTOM_JOINTS_API CustomControlledBallAndSocket(const dMatrix& pinAndPivotFrame, NewtonBody* const child, NewtonBody* const parent = NULL);
	CUSTOM_JOINTS_API virtual ~CustomControlledBallAndSocket();

	CUSTOM_JOINTS_API void SetAngularVelocity (dFloat omegaMag);
	CUSTOM_JOINTS_API dFloat GetAngularVelocity () const;

	CUSTOM_JOINTS_API void SetPitchAngle (dFloat angle);
	CUSTOM_JOINTS_API dFloat SetPitchAngle () const;

	CUSTOM_JOINTS_API void SetYawAngle (dFloat angle);
	CUSTOM_JOINTS_API dFloat SetYawAngle () const;

	CUSTOM_JOINTS_API void SetRollAngle (dFloat angle);
	CUSTOM_JOINTS_API dFloat SetRollAngle () const;

	protected:
	CUSTOM_JOINTS_API void GetInfo (NewtonJointRecord* const info) const;
	CUSTOM_JOINTS_API virtual void SubmitConstraints (dFloat timestep, int threadIndex);

	dVector m_targetAngles;
	AngularIntegration m_pitch;
	AngularIntegration m_yaw;
	AngularIntegration m_roll;
	dFloat m_angulaSpeed;
};

#endif 

