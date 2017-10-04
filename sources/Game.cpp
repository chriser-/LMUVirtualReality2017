#include "Game.h"
#include "bird.h"
#include <OSGComponentTransform.h>
#include <OSGComponentTransformBase.h>
#include <OpenSG/OSGSceneGraphUtils.h>  // header for SceneGraphPrinter
#include <OSGSimpleGeometry.h>
#include <inVRs/SystemCore/UserDatabase/UserDatabase.h>


OSG_USING_STD_NAMESPACE
OSG_USING_NAMESPACE

void Game::AddBehavior(GameObject* behavior)
{
	std::cout << "Adding Behavior " << GameObject::hash_value(*behavior) << std::endl;
	m_behaviors.insert(behavior);
	// add behavior to root node
	m_root.node()->addChild(behavior->GetTransform().node());
}

void Game::RemoveBehavior(GameObject* behavior)
{
	std::cout << "Removing Behavior " << GameObject::hash_value(*behavior) << std::endl;
	m_behaviors.erase(behavior);
	const NodeRecPtr nodeToDelete = behavior->GetTransform().node();
	m_root.node()->subChild(nodeToDelete);
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
	struct BirdPosInfo
	{
		std::string Name;
		std::string SpriteName;
		Vec3f Position;
	};
	BirdPosInfo birds[] = {
		{"Bird 1", "DuckBlue", Vec3f(0,100,-10)},
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
	std::cout << "local user " << localUser << std::endl;
	CameraTransformation* camera = localUser->getCamera();
	std::cout << "camera " << camera << std::endl;

	std::string skyPath = "skybox/";
	m_skybox.init(5, 5, 5, 1000, (skyPath + "lostatseaday/lostatseaday_dn.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_up.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_ft.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_bk.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_rt.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_lf.jpg").c_str());
	m_root.node()->addChild(m_skybox.getNodePtr());
	m_skybox.setupRender(camera->getPosition());

	// trees
	GameObject* tree = new GameObject("Tree");
	tree->GetTransform()->setTranslation(Vec3f(0, 0, -10));
	tree->GetTransform()->setScale(Vec3f(0.5, 0.5, 0.5));
	Sprite* treeSprite = new Sprite(tree->GetTransform().node(), "Tree", "objects", -2);
	tree->AddComponent(treeSprite);
	Vec2f dimensions = treeSprite->GetDimensions();
	tree->Translate(Vec3f(0, dimensions.y()/2, 0));

	// grass
	GameObject* grass = new GameObject("Grass");
	grass->GetTransform()->setTranslation(Vec3f(0, 0, -10));
	grass->GetTransform()->setScale(Vec3f(0.25, 0.25, 0.25));
	Sprite* grassSprite = new Sprite(grass->GetTransform().node(), "Ground", "objects", -1);
	grass->AddComponent(grassSprite);
	dimensions = grassSprite->GetDimensions();
	grass->Translate(Vec3f(0, dimensions.y() / 2, 0));

	// debug wand display
	m_debugWand = ComponentTransformNodeRefPtr::create();
	m_debugWand->setScale(Vec3f(1, 1, 1));
	GeometryNodeRefPtr wandGeo = GeometryNodeRefPtr::create();
	wandGeo.node()->setCore(makeCylinderGeo(5000, 3, 100, true, true, true).get());
	m_debugWand.node()->addChild(wandGeo);
	m_root.node()->addChild(m_debugWand);
}

void Game::Update()
{
	// Update all behaviors
	for (auto& behavior : m_behaviors)
	{
		behavior->Update();
	}
}

void Game::UpdateWand(Vec3f position, Quaternion orientation) const
{
	m_debugWand->setTranslation(position);
	m_debugWand->setRotation(orientation);
}

Game::~Game()
{
	// Delete all behaviors
	for (auto& behavior : m_behaviors)
	{
		delete behavior;
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
