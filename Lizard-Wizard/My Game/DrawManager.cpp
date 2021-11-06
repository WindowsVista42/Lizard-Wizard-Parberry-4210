#include "Game.h"
#include "Defines.h"

void CGame::DrawDebugModelsOnRB() {
    //NOTE(sean): the reason batching all of this together works, is that we're doing all the vertex calculations on the cpu instead of the gpu
    m_pRenderer->BeginDebugLineBatch();
    {
        for every(j, m_pDynamicsWorld->getNumCollisionObjects()) {
            btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[j];
            btCollisionShape* shape = obj->getCollisionShape();
            btRigidBody* body = btRigidBody::upcast(obj);
            btTransform trans;

            if (body && body->getMotionState()) {
                body->getMotionState()->getWorldTransform(trans);
            }
            else {
                trans = obj->getWorldTransform();
            }

            switch (shape->getShapeType()) {
            case(BT_SHAPE_TYPE_BOX): {
                //NOTE(sean): render box
                btBoxShape* castratedObject = reinterpret_cast<btBoxShape*>(shape);
                BoundingBox box = BoundingBox(*(Vector3*)&trans.getOrigin(), *(Vector3*)&(castratedObject->getHalfExtentsWithMargin()));
                m_pRenderer->DrawDebugAABB(box, Colors::Red);

                //NOTE(sean): render grid on box
                Vector3 ext = *(Vector3*)&castratedObject->getHalfExtentsWithMargin();
                Vector3 x_axis(ext.x, 0, 0);
                Vector3 y_axis(0.0, 0.0, ext.z);
                Vector3 origin = *(Vector3*)&trans.getOrigin();
                origin.y += ext.y;

                m_pRenderer->DrawDebugGrid(x_axis, y_axis, origin, 4, 4, Colors::GreenYellow);
            } break;

            case(BT_SHAPE_TYPE_CAPSULE): {
                btCapsuleShape* castratedObject = reinterpret_cast<btCapsuleShape*>(shape);
                m_pRenderer->DrawDebugCapsule(*(Vector3*)&trans.getOrigin(), castratedObject->getRadius(), castratedObject->getHalfHeight(), 32, Colors::Purple);
            } break;

            default: {
                btSphereShape* castratedObject = reinterpret_cast<btSphereShape*>(shape);
                BoundingSphere sphere = BoundingSphere(*(Vector3*)&trans.getOrigin(), castratedObject->getRadius());
                m_pRenderer->DrawDebugSphere(sphere, 16, Colors::CadetBlue);
            } break;
            }
        }

        for every(j, m_currentRayProjectiles.size()) {
            //m_pRenderer->DrawDebugRay(m_currentRayProjectiles[j].Pos1, m_currentRayProjectiles[j].Pos2, 50000, m_currentRayProjectiles[j].Color);
            m_pRenderer->DrawDebugLine(m_currentRayProjectiles[j].Pos1, m_currentRayProjectiles[j].Pos2, m_currentRayProjectiles[j].Color);
        }
    }
    m_pRenderer->EndDebugBatch();
}
