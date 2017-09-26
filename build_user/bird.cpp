#include <OSGRefCountPtr.h>
#include <OSGNode.h>
#include <OSGGroup.h>
#include <OSGTransform.h>
#include <OSGGeometryBase.h>
#include <OSGSimpleGeometry.h>
#include <OSGComponentTransform.h>
#include <OSGComponentTransformBase.h>
#include "Behavior.h"

OSG_USING_NAMESPACE
OSG_USING_STD_NAMESPACE

NodeTransitPtr makeBird()
{
	NodeRecPtr rootBird = Node::create();

	// Create Transform node for the bird
	ComponentTransformRecPtr birdTransform = ComponentTransform::create();
	rootBird->setCore(birdTransform);
	// root node is now set as TranformRecPtr

	// create geometry node for the bird as child of the root
	GeometryRecPtr birdGeo = makeBoxGeo(2,2,2,1,1,1); // TODO: use sprite
	NodeRecPtr birdGeoNode = Node::create();
	birdGeoNode->setCore(birdGeo);
	rootBird->addChild(birdGeoNode);
	// geoNode is now set as geometry and geoNode is added as child to the root transform

	// Bird (Transform)
	//  |
	// Bird (Geometry)

	return NodeTransitPtr(rootBird);
}

class Bird : Behavior
{
private:
	NodeTransitPtr m_bird;
	ComponentTransformRecPtr m_birdTransform;
	Vec3f m_velocity;
	static const int START_VELOCITY_COUNT;
	static const Vec3f START_VELOCITIES[];
public:
	Bird();
	void Update();
};

const int Bird::START_VELOCITY_COUNT = 2;
const Vec3f Bird::START_VELOCITIES[START_VELOCITY_COUNT] = {
	Vec3f(1,1,0),
	Vec3f(-1,1,0)
};


Bird::Bird()
{
	m_bird = makeBird();
	m_birdTransform = dynamic_cast<ComponentTransform*>(m_bird->getCore());
	m_velocity = START_VELOCITIES[rand() % START_VELOCITY_COUNT];
}

void Bird::Update(float deltaTime)
{
	if(m_birdTransform != nullptr)
	{
		const Vec3f currentTranslation = m_birdTransform->getTranslation();
		m_birdTransform->setTranslation(currentTranslation + m_velocity);
	}
}
