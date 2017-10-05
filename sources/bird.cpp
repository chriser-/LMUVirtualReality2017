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

const int Bird::START_VELOCITY_COUNT = 3;
const Vec3f Bird::START_VELOCITIES[START_VELOCITY_COUNT] = {
	Vec3f(1,1,0),
	Vec3f(-1,1,0),
	Vec3f(0,1,0)
};
//									  y  x
const string Bird::VELOCITY_TO_SPRITE[3][3] = {
	//-1			0			1
	{ "left_up",	"up",	"right_up"	},		// 1
	{ "left",		"",		"right"		},		// 0
	{ "left",		"",		"right"		},		// -1
};

float getNextDirectionTimer()
{
	return 2.f + ((rand() % 40) / 10.f); // 2 + (0..4) seconds
}

Bird::Bird(std::string name, std::string spriteName) : GameObject(name)
{
	//m_Transform->setTranslation(Vec3f(0,100,-10));
	m_direction = START_VELOCITIES[rand() % START_VELOCITY_COUNT];
	m_speed = 1;
	m_hitSpriteTimer = 0.5f;
	m_nextDirectionTimer = getNextDirectionTimer();
	m_hit = m_dead = false;
	m_BirdSprite = new Sprite(GetTransform().node(), spriteName, "units");
	m_BirdSprite->SetTimePerFrame(0.05);
	AddComponent(m_BirdSprite);
	UpdateDirectionSprite();
}

void Bird::UpdateDirectionSprite() const
{
	// update sprite to match direction
	std::string newSprite = VELOCITY_TO_SPRITE[abs(int(m_direction.y()) - 1)][abs(int(m_direction.x()) + 1)];
	//std::cout << "New Sprite is " << newSprite << "[" << int(newXDirection) + 1 << "][" << int(newYDirection) + 1 << "]" << std::endl;
	m_BirdSprite->UpdateCurrentSprite(newSprite);
}

void Bird::Update()
{
	GameObject::Update();
	if(m_Transform.core() != nullptr)
	{
		if(m_hit)
		{
			m_hitSpriteTimer -= MyTime::DeltaTime;
			if (m_hitSpriteTimer > 0)
				return;
			m_dead = true;
			m_BirdSprite->UpdateCurrentSprite("dead", 1);
		}
		float xPosition, yPosition, zPosition;
		m_Transform->getTranslation().getSeparateValues(xPosition, yPosition, zPosition);
		if (!m_dead)
		{
			// keep position in bounds
			float newXDirection, newYDirection, newZDirection;
			m_direction.getSeparateValues(newXDirection, newYDirection, newZDirection);
			// change direction after certain time
			m_nextDirectionTimer -= MyTime::DeltaTime * (m_speed / 30.f);
			if(m_nextDirectionTimer <= 0)
			{
				newXDirection = (rand() % 3) - 1;
				newYDirection = (rand() % 3) - 1;
			}
			if (xPosition > 200)
			{
				newXDirection = -1;
			}
			else if (xPosition < -200)
			{
				newXDirection = 1;
			}
			if (yPosition > 200)
			{
				newYDirection = -1;
			}
			else if (yPosition < 100)
			{
				newYDirection = 1;
			}

			if (newXDirection == 0 && (newYDirection == 0 || newYDirection == -1)) // center or down are not existing
				newXDirection = 1; // fallback: move to right

			if (m_direction.x() != newXDirection || m_direction.y() != newYDirection)
			{
				m_nextDirectionTimer = getNextDirectionTimer();

				//std::cout << "New Direction is (" << newXDirection << "," << newYDirection << ") at speed " << m_speed << std::endl;
				m_direction.setValues(newXDirection, newYDirection, newZDirection);
				UpdateDirectionSprite();
			}
		}
		else
		{
			if(yPosition < -20)
			{
				delete this;
				return;
			}
		}
		// translate into velocity direction with speed
        Translate(m_direction * m_speed * MyTime::DeltaTime);
		//std::cout << "Position is (" << m_Transform->getTranslation().x() << "," << m_Transform->getTranslation().y() << ")" << std::endl;
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

void Bird::OnHit()
{
	if (!m_hit)
	{
		std::cout << "Bird was hit!" << std::endl;
		m_hit = true;
		m_BirdSprite->UpdateCurrentSprite("hit", 1);
		m_speed = 100;
		m_direction = Vec3f(0, -1, 0);
	}
}

Bird::~Bird()
{
	
}
