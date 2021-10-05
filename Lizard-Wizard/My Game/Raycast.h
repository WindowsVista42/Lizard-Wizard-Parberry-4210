///btBulletDynamicsCommon.h is the main Bullet include file, contains most common include files.
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"

#include <stdio.h>  //printf debugging

#include "LinearMath/btAlignedObjectArray.h"

///RaytestDemo shows how to use the btCollisionWorld::rayTest feature

// Bullet3 Inclusions
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
//#include <CommonRigidBodyBase.h>

class RaytestDemo //: public CommonRigidBodyBase
{
public:
	RaytestDemo(struct GUIHelperInterface* helper)
		//: CommonRigidBodyBase(helper)
	{
	}
	virtual ~RaytestDemo()
	{
	}
	virtual void initPhysics();

	virtual void exitPhysics();

	void castRays();

	virtual void stepSimulation(float deltaTime);

	virtual void resetCamera()
	{
		float dist = 18;
		float pitch = -30;
		float yaw = 129;
		float targetPos[3] = { -4.6, -4.7, -5.75 };
		//m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}//#pragma once
};