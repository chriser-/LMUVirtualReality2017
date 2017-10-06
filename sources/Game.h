#pragma once
#include <OSGNode.h>
#include "GameObject.h"
#include <boost/unordered_map.hpp>
#include "SpriteAtlas.h"
#include <OSGGroup.h>
#include <inVRs/tools/libraries/Skybox/Skybox.h>
#include "Gun.h"

class Game
{
private:
	boost::unordered_map<OSG::Node*, GameObject*> m_behaviors;
	boost::unordered_set<OSG::Node*> m_behaviorsToDelete;
	std::unordered_map<std::string, SpriteAtlas*> m_SpriteAtlasses;
	OSG::ComponentTransformNodeRefPtr m_root;
	Skybox m_skybox;
	Gun* m_gun;
	int m_currentWave;
	bool m_gameReady;
	int m_birdsAlive;
	static Game* m_gameInstance;
public:
	Game();
    void Update();
	void UpdateWand(OSG::Vec3f position, OSG::Quaternion orientation) const;
	~Game();
	void Scroll(int direction) const;
	OSG::NodeTransitPtr GetRootNode() const;
	static Game* Instance();
	void AddBehavior(GameObject* behavior);
	void RemoveBehavior(GameObject* behavior);
	Gun* GetGun();
	GameObject* GetBehavior(OSG::NodeRecPtr fromNode);
	SpriteAtlas* GetSpriteAtlas(std::string spriteAtlasName);
};
