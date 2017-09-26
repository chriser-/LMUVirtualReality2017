#include "Game.h"
#include "bird.h"
#include <OSGComponentTransform.h>
#include <OSGComponentTransformBase.h>
#include <OpenSG/OSGSceneGraphUtils.h>  // header for SceneGraphPrinter

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
	SpriteAtlas* atlas = new SpriteAtlas("units");
	m_SpriteAtlasses[spriteAtlasName] = atlas;
	return atlas;
}

Game* Game::m_gameInstance;
Game::Game()
{
	m_gameInstance = this;
	m_root = GroupNodeRefPtr::create();
	SpriteAtlas* atlas = GetSpriteAtlas("units");
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
		std::cout << "Creating " << bird.Name << std::endl;
		Bird* b = new Bird(bird.Name, bird.SpriteName);
		b->GetTransform()->setTranslation(bird.Position);
		b->SetSpeed(30);
	}
}

void Game::Update()
{
	// Update all behaviors
	for (auto& behavior : m_behaviors)
	{
		behavior->Update();
	}
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
