#include <OSGRefCountPtr.h>
#include <OSGNode.h>
#include <OSGGroup.h>
#include <OSGTransform.h>
#include <OSGGeometryBase.h>
#include <OSGSimpleGeometry.h>
#include <OSGComponentTransform.h>
#include <OSGComponentTransformBase.h>
#include "bird.h"
#include "Time.h"
#include "Game.h"

OSG_USING_NAMESPACE
OSG_USING_STD_NAMESPACE

ComponentTransformNodeRefPtr makeBird()
{
	// Create Transform node for the bird
	ComponentTransformNodeRefPtr birdTransform = ComponentTransformNodeRefPtr::create();
	// root node is now set as TranformRecPtr

	// create geometry node for the bird as child of the root
	GeometryNodeRefPtr birdGeoNode = GeometryNodeRefPtr::create();
	birdGeoNode.node()->setCore(makePlaneGeo(25, 25, 1, 1));
	birdTransform.node()->addChild(birdGeoNode);
	// geoNode is now set as geometry and geoNode is added as child to the root transform

	// Bird (Transform)
	//  |
	// Bird (Geometry)

	return birdTransform;
}

const int Bird::START_VELOCITY_COUNT = 2;
const Vec3f Bird::START_VELOCITIES[START_VELOCITY_COUNT] = {
	Vec3f(1,1,0),
	Vec3f(-1,1,0)
};
const string Bird::VELOCITY_TO_SPRITE[3][3] = {
	//-1			0			1
	{ "left_down",	"left",	"left_up"},	// -1
	{ "left",		"",		"right"},		// 0
	{ "right_down",	"right","right_up"}		// 1
};

Bird::Bird(std::string name, std::string spriteName) : GameObject(name)
{
	//m_Transform->setTranslation(Vec3f(0,100,-10));
	m_direction = START_VELOCITIES[rand() % START_VELOCITY_COUNT];
	m_speed = 1;
	m_BirdSprite = new Sprite(GetTransform().node(), spriteName, "units");
	AddComponent(m_BirdSprite);
}

void Bird::Update()
{
	GameObject::Update();
	if(m_Transform.core() != nullptr)
	{
		// keep position in bounds
		float newXDirection, newYDirection, newZDirection;
		float xPosition, yPosition, zPosition;
		m_direction.getSeparateValues(newXDirection, newYDirection, newZDirection);
		m_Transform->getTranslation().getSeparateValues(xPosition, yPosition, zPosition);
		if(xPosition > 200)
		{
			newXDirection = -1;
		}
		else if(xPosition < -200)
		{
			newXDirection = 1;
		}
		if(yPosition > 200)
		{
			newYDirection = -1;
		}
		else if(yPosition < 0)
		{
			newYDirection = 1;
		}
		std::cout << "New Direction is (" << newXDirection << "," << newYDirection << ") at speed " << m_speed << std::endl;
		m_direction.setValues(newXDirection, newYDirection, newZDirection);

		// update sprite to match velocity
		std::string newSprite = VELOCITY_TO_SPRITE[int(newXDirection) +1][int(newYDirection) +1];
		std::cout << "New Sprite is " << newSprite << "[" << int(newXDirection) + 1 << "][" << int(newYDirection) + 1 << "]" << std::endl;
		m_BirdSprite->UpdateCurrentSprite(newSprite);

		// translate into velocity direction with speed
		const Vec3f currentTranslation = m_Transform->getTranslation();
		m_Transform->setTranslation(currentTranslation + (m_direction * m_speed * Time::DeltaTime));
		std::cout << "Position is (" << m_Transform->getTranslation().x() << "," << m_Transform->getTranslation().y() << ")" << std::endl;
	}
}

void Bird::SetSpeed(float speed)
{
	m_speed = speed;
}

void Bird::SetSprite(Sprite* sprite)
{
	m_BirdSprite = sprite;
	AddComponent(m_BirdSprite);
}

Bird::~Bird()
{
	
}
