#pragma once
#include "Component.h"
#include <OSGSimpleTexturedMaterialBase.h>
#include <OSGSwitch.h>
#include <unordered_map>
#include "SpriteAtlas.h"
#include "Game.h"
#include <OSGGeometry.h>
#include <OSGSimpleGeometry.h>


class Sprite : public Component
{
private:
	OSG::SwitchNodeRefPtr m_SpriteSwitcher;
	std::string m_Name;
	std::unordered_map<std::string, int> m_SpriteIdToIndexMap;
	std::unordered_map<std::string, std::vector<std::string>> m_SpriteList;
	std::string m_CurrentSpriteKey;
	bool m_forceSpriteUpdate;
	int m_CurrentFrame;
	float m_TimeUntilNextFrame;
	float m_TimePerFrame;
	SpriteAtlas* m_SpriteAtlas;
public:
	Sprite(NodeRecPtr parentNode, std::string name, std::string spriteAtlas, int sortKey = 0);
	void SetSprite(std::string key, int frame);
	void UpdateCurrentSprite(std::string key, int frame = -1);
	void SetTimePerFrame(float timePerFrame);
	Vec2f GetDimensions() const;
	~Sprite() override;
	void Update() override;
};
