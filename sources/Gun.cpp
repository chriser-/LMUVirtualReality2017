#include "Gun.h"
#include "GameObject.h"
#include "Mesh.h"
#include <OSGSimpleGeometry.h>
#include <OSGNameAttachment.h>
#include <OSGIntersectAction.h>
#include "Game.h"
#include "Sprite.h"

Gun::Gun() : GameObject("Gun")
{
	m_remainingShots = 0;
	// mesh of the gun
	GameObject* gunMesh = new GameObject("GunMesh");
	Mesh* mesh = new Mesh(gunMesh->GetTransform().node(), "zapper");
	gunMesh->AddComponent(mesh);

	OSG::Quaternion rot;
	rot.setValue(OSG::osgDegree2Rad(30), OSG::osgDegree2Rad(180), 0);
	gunMesh->GetTransform()->setRotation(rot);
	gunMesh->GetTransform()->setScale(OSG::Vec3f(10, 10, 10));

	AddChild(gunMesh);

	// barrel exit position
	m_barrelExitPosition = new GameObject("BarrelExitPosition");
	AddChild(m_barrelExitPosition);
	m_barrelExitPosition->Translate(OSG::Vec3f(0, -4, -17));
	m_barrelExitPosition->GetTransform()->setRotation(rot);

	// debug geo to visualize the exit position
	//OSG::GeometryNodeRefPtr barrellExitGeo = OSG::GeometryNodeRefPtr::create();
	//barrellExitGeo.node()->setCore(OSG::makeSphereGeo(5, 5).get());
	//m_barrelExitPosition->GetTransform().node()->addChild(barrellExitGeo);


	//m_lineStart = OSG::ComponentTransformNodeRefPtr::create();
	//OSG::GeometryNodeRefPtr lineStartGeo = OSG::GeometryNodeRefPtr::create();
	//lineStartGeo.node()->setCore(OSG::makeSphereGeo(5, 5).get());
	//m_lineStart.node()->addChild(lineStartGeo);
	//Game::Instance()->GetRootNode()->addChild(m_lineStart);

	//m_lineEnd = OSG::ComponentTransformNodeRefPtr::create();
	//OSG::GeometryNodeRefPtr lineEndGeo = OSG::GeometryNodeRefPtr::create();
	//lineEndGeo.node()->setCore(OSG::makeSphereGeo(5, 5).get());
	//m_lineEnd.node()->addChild(lineEndGeo);
	//Game::Instance()->GetRootNode()->addChild(m_lineEnd);

	// this is the direction of the barrel
	/*OSG::ComponentTransformNodeRefPtr barrelExitShot = OSG::ComponentTransformNodeRefPtr::create();
	OSG::GeometryNodeRefPtr barrelExitShotGeo = OSG::GeometryNodeRefPtr::create();
	barrelExitShotGeo.node()->setCore(OSG::makeCylinderGeo(50, 0.3, 100, true, true, true).get());
	barrelExitShot->setRotation(OSG::Quaternion(OSG::Vec3f(1,0,0), OSG::osgDegree2Rad(90)));
	barrelExitShot->setTranslation(OSG::Vec3f(0, 0, 25));
	barrelExitShot.node()->addChild(barrelExitShotGeo);
	m_barrelExitPosition->GetTransform().node()->addChild(barrelExitShot);*/
	

	// this is the orientation of the wand in real world
	//OSG::ComponentTransformNodeRefPtr wandChildDebug = OSG::ComponentTransformNodeRefPtr::create();
	//wandChildDebug->setRotation(OSG::Quaternion(OSG::Vec3f(1, 0, 0), OSG::osgDegree2Rad(-90)));
	//OSG::GeometryNodeRefPtr wandGeo = OSG::GeometryNodeRefPtr::create();
	//wandGeo.node()->setCore(OSG::makeCylinderGeo(500, 2, 100, true, true, true).get());
	//wandChildDebug.node()->addChild(wandGeo);
	//GetTransform().node()->addChild(wandChildDebug);
	
	// crosshair
	m_crosshair = new GameObject("Crosshair");
	m_crosshair->GetTransform()->setScale(OSG::Vec3f(0.2, 0.2, 0.2));
	m_crosshair->AddComponent(new Sprite(m_crosshair->GetTransform().node(), "Crosshair", "objects", 999));
	Game::Instance()->GetRootNode()->addChild(m_crosshair->GetTransform().node());

	// remaining shots display
}

void Gun::Update()
{
	GameObject::Update();
	OSG::Line ray = CalculateRay();
	//m_lineStart->setTranslation(OSG::Vec3f(ray.getPosition()));
	// get point where z = -100
	float length = ray.getPosition().z() + 100;
	OSG::Vec3f newCrosshairPosition = OSG::Vec3f(ray.getPosition()) + length * ray.getDirection();
	//m_lineEnd->setTranslation(newCrosshairPosition);
	m_crosshair->GetTransform()->setTranslation(newCrosshairPosition);
}


OSG::Line Gun::CalculateRay()
{
	OSG::Line l;

	// point 1 is the position of the barrel exit position
	OSG::Vec3f point1;
	m_Transform->getRotation().multVec(m_barrelExitPosition->GetTransform()->getTranslation(), point1);
	point1 += m_Transform->getTranslation();
	// point 2 is the position+direction of the wand
	OSG::Vec3f point2;
	OSG::Quaternion lookDirection = m_Transform->getRotation() * m_barrelExitPosition->GetTransform()->getRotation();
	lookDirection.multVec(OSG::Vec3f(0, 0, 1), point2);
	point2 *= 5000; // length of 5000
	point2 += point1;
	l.setValue(point1, point2);

	return l;
}

void Gun::Shoot()
{
	OSG::IntersectActionRefPtr ray = OSG::IntersectAction::create();
	ray->setLine(CalculateRay());
	// apply on bird root node
	ray->apply(Game::Instance()->GetRootNode()->getChild(0));

	// did we hit something?
	if (ray->didHit())
	{
		//std::string hitNodeName = "N/A";
		OSG::NodeRecPtr hitNode = ray->getHitObject();
		while (hitNode != nullptr)
		{
			//if (getName(hitNode))
			//{
			//	hitNodeName = OSG::getName(hitNode);
			//}
			GameObject* hitObject = Game::Instance()->GetBehavior(hitNode);
			std::cout << " object " << hitNode
				<< " type " << hitNode->getCore()->getTypeName()
				//<< " name " << hitNodeName
				<< " pos " << ray->getHitPoint()
				<< std::endl;
			if (hitObject == nullptr)
			{
				hitNode = hitNode->getParent();
			}
			else
			{

				hitObject->OnHit();
				break;
			}
		}
	}
	else
	{
		m_remainingShots--;
		std::cout << "nothing was hit. remaining shots " << m_remainingShots << std::endl;
		if(m_remainingShots <= 0)
		{
			std::cout << "game over" << std::endl;
		}
	}

	// free the action
	ray = nullptr;
}

void Gun::SetRemainingShots(int shots)
{
	m_remainingShots = shots;
}

GameObject* Gun::GetBarrelExitGameObject()
{
	return m_barrelExitPosition;
}
