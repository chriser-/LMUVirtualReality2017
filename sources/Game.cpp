#include "Game.h"
#include "bird.h"
#include <OSGComponentTransform.h>
#include <OSGComponentTransformBase.h>
#include <OpenSG/OSGSceneGraphUtils.h>  // header for SceneGraphPrinter
#include <OSGSimpleGeometry.h>
#include <OSGNameAttachment.h>
#include <inVRs/SystemCore/UserDatabase/UserDatabase.h>


OSG_USING_STD_NAMESPACE
OSG_USING_NAMESPACE


void Game::AddBehavior(GameObject* behavior)
{
	std::cout << "Adding Behavior " << behavior->GetName() << " " << GameObject::hash_value(*behavior) << " (node: " << behavior->GetTransform().node() << ")" << std::endl;
	m_behaviors[behavior->GetTransform().node()] = behavior;
	// add behavior to root node
	if(behavior->GetName().compare("Bird") == 0)
	{
		std::cout << "bird to 2nd group" << std::endl;
		m_root.node()->getChild(0)->addChild(behavior->GetTransform().node());
	}
	else
	{
		m_root.node()->addChild(behavior->GetTransform().node());
	}
}

void Game::RemoveBehavior(GameObject* behavior)
{
	std::cout << "Removing Behavior " << GameObject::hash_value(*behavior) << std::endl;
	m_behaviors.erase(behavior->GetTransform().node());
	const NodeRecPtr nodeToDelete = behavior->GetTransform().node();
	if (behavior->GetName() == "Bird")
	{
		m_root.node()->getChild(0)->subChild(nodeToDelete);
	}
	else
	{
		m_root.node()->subChild(nodeToDelete);
	}
}

GameObject* Game::GetBehavior(NodeRecPtr fromNode)
{
	const auto iter = m_behaviors.find(fromNode);
	if(iter != m_behaviors.end())
	{
		return iter->second;
	}
	return nullptr;
}

SpriteAtlas* Game::GetSpriteAtlas(std::string spriteAtlasName)
{
	const auto iter = m_SpriteAtlasses.find(spriteAtlasName);
	if(iter != m_SpriteAtlasses.end())
	{
		return iter->second;
	}
	SpriteAtlas* atlas = new SpriteAtlas(spriteAtlasName);
	m_SpriteAtlasses[spriteAtlasName] = atlas;
	return atlas;
}

Game* Game::m_gameInstance;
Game::Game()
{
	m_gameInstance = this;
	m_root = GroupNodeRefPtr::create();
	m_root.node()->addChild(GroupNodeRefPtr::create()); // bird root (hack for collision)
	setName(m_root.node(), "Root");
	struct BirdPosInfo
	{
		std::string Name;
		std::string SpriteName;
		Vec3f Position;
	};
	BirdPosInfo birds[] = {
		{"Bird", "DuckBlue", Vec3f(0, 100, -100)},
		//{"Bird 2", "DuckBlue", Vec3f(0,100,-10)},
		//{"Bird 3", "DuckBlue", Vec3f(0,100,-10)},
	};
	for (auto bird : birds)
	{
		//std::cout << "Creating " << bird.Name << std::endl;
		Bird* b = new Bird(bird.Name, bird.SpriteName);
		b->GetTransform()->setTranslation(bird.Position);
		b->SetSpeed(30);
	}

	// floor
	GameObject* floor = new GameObject("Floor");
	GeometryNodeRefPtr floorGeo = GeometryNodeRefPtr::create();
	floorGeo.node()->setCore(makePlaneGeo(5000, 5000, 1, 1));
	floor->GetTransform().node()->addChild(floorGeo);
	floor->GetTransform()->setRotation(Quaternion(Vec3f(1, 0, 0), osgDegree2Rad(90)));

	// skybox
	User* localUser = UserDatabase::getLocalUser();
	CameraTransformation* camera = localUser->getCamera();

	std::string skyPath = "skybox/";
	m_skybox.init(20, 20, 20, 10000, (skyPath + "lostatseaday/lostatseaday_dn.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_up.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_ft.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_bk.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_rt.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_lf.jpg").c_str());
	m_root.node()->addChild(m_skybox.getNodePtr());
	m_skybox.setupRender(camera->getPosition());

	// trees
	GameObject* tree = new GameObject("Tree");
	tree->GetTransform()->setTranslation(Vec3f(0, 0, -99));
	tree->GetTransform()->setScale(Vec3f(0.5, 0.5, 0.5));
	Sprite* treeSprite = new Sprite(tree->GetTransform().node(), "Tree", "objects", -2);
	tree->AddComponent(treeSprite);
	Vec2f dimensions = treeSprite->GetDimensions();
	tree->Translate(Vec3f(0, dimensions.y()/2, 0));

	// grass
	GameObject* grass = new GameObject("Grass");
	grass->GetTransform()->setTranslation(Vec3f(0, 0, -101));
	grass->GetTransform()->setScale(Vec3f(0.25, 0.25, 0.25));
	Sprite* grassSprite = new Sprite(grass->GetTransform().node(), "Ground", "objects", -1);
	grass->AddComponent(grassSprite);
	dimensions = grassSprite->GetDimensions();
	grass->Translate(Vec3f(0, dimensions.y() / 2, 0));

	
	// debug wand display
	m_debugWand = ComponentTransformNodeRefPtr::create();
	ComponentTransformNodeRefPtr wandChild = ComponentTransformNodeRefPtr::create();
	wandChild->setRotation(Quaternion(Vec3f(1, 0, 0), osgDegree2Rad(-90)));
	GeometryNodeRefPtr wandGeo = GeometryNodeRefPtr::create();
	wandGeo.node()->setCore(makeCylinderGeo(500, 2, 100, true, true, true).get());
	wandChild.node()->addChild(wandGeo);
	m_debugWand.node()->addChild(wandChild);
	m_root.node()->addChild(m_debugWand);
	
}

void Game::Update()
{
	// Update all behaviors
	for (auto& behavior : m_behaviors)
	{
		behavior.second->Update();
	}
}

void Game::UpdateWand(Vec3f position, Quaternion orientation) const
{
	m_debugWand->setTranslation(Vec3f(position.x(), position.y(), position.z()));
	m_debugWand->setRotation(orientation);
}

Game::~Game()
{
	// Delete all behaviors
	for (auto& behavior : m_behaviors)
	{
		delete behavior.second;
	}
}

void Game::Scroll(int direction) const
{
	
}

NodeTransitPtr Game::GetRootNode() const
{
	return NodeTransitPtr(m_root.node());
}

Game* Game::Instance()
{
	return m_gameInstance;
}
