#pragma once
#include "Component.h"
#include <OSGSimpleTexturedMaterialBase.h>
#include <OSGSwitch.h>
#include <unordered_map>
#include "SpriteAtlas.h"
#include "Game.h"
#include <OSGGeometry.h>
#include <OSGSimpleGeometry.h>

OSG_USING_NAMESPACE

class Sprite : public Component
{
private:
	SwitchNodeRefPtr m_SpriteSwitcher;
	std::string m_Name;
	std::unordered_map<std::string, int> m_SpriteIdToIndexMap;
	std::unordered_map<std::string, std::vector<std::string>> m_SpriteList;
	std::string m_CurrentSpriteKey;
	int m_CurrentFrame;
	float m_TimeUntilNextFrame;
	float m_TimePerFrame;
	SpriteAtlas* m_SpriteAtlas;
public:
	Sprite(NodeRecPtr parentNode, std::string name, std::string spriteAtlas);
	void SetSprite(std::string key, int frame);
	void UpdateCurrentSprite(std::string key);
	void SetTimePerFrame(float timePerFrame);
	~Sprite() override;
	void Update() override;
};
