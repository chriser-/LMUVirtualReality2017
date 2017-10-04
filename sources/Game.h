﻿#pragma once
#include <OSGNode.h>
#include "GameObject.h"
#include <boost/unordered_map.hpp>
#include "SpriteAtlas.h"
#include <OSGGroup.h>
#include <inVRs/tools/libraries/Skybox/Skybox.h>

class Game
{
private:
	boost::unordered_map<Node*, GameObject*> m_behaviors;
	boost::unordered_set<Node*> m_behaviorsToDelete;
	std::unordered_map<std::string, SpriteAtlas*> m_SpriteAtlasses;
	OSG::GroupNodeRefPtr m_root;
	Skybox m_skybox;
	static Game* m_gameInstance;
	void BirdDie();
public:
	ComponentTransformNodeRefPtr m_debugWand;
	Game();
    void Update();
	void UpdateWand(Vec3f position, Quaternion orientation) const;
	~Game();
	void Scroll(int direction) const;
	OSG::NodeTransitPtr GetRootNode() const;
	static Game* Instance();
	void AddBehavior(GameObject* behavior);
	void RemoveBehavior(GameObject* behavior);
	GameObject* GetBehavior(NodeRecPtr fromNode);
	SpriteAtlas* GetSpriteAtlas(std::string spriteAtlasName);
};
