#pragma once
#include <OSGNode.h>
#include "GameObject.h"
#include <boost/unordered_set.hpp>
#include "SpriteAtlas.h"

class Game
{
private:
	boost::unordered_set<GameObject*> m_behaviors;
	std::unordered_map<std::string, SpriteAtlas*> m_SpriteAtlasses;
	GroupNodeRefPtr m_root;
	static Game* m_gameInstance;
public:
	Game();
    void Update();
	~Game();
	void Scroll(int direction) const;
	NodeTransitPtr GetRootNode() const;
	static Game* Instance();
	void AddBehavior(GameObject* behavior);
	void RemoveBehavior(GameObject* behavior);
	SpriteAtlas* GetSpriteAtlas(std::string spriteAtlasName);
};
