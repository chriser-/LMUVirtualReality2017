#include "Sprite.h"
#include <OSGSimpleTexturedMaterial.h>
#include <OSGImage.h>
#include <OSGSwitch.h>
#include "Game.h"
#include <OSGGeometry.h>
#include <OSGSimpleGeometry.h>
#include "Time.h"

// key -> sprite names for each frame
Sprite::Sprite(NodeRecPtr parentNode, std::string name, std::string spriteAtlas)
{
	m_Name = name;
	m_SpriteAtlas = Game::Instance()->GetSpriteAtlas(spriteAtlas);
	m_SpriteSwitcher = SwitchNodeRefPtr::create();
	m_Node = m_SpriteSwitcher;
	std::cout << "Adding sprite switcher (" << m_SpriteSwitcher.node() << " to node " << parentNode << std::endl;
	parentNode->addChild(m_SpriteSwitcher);
	m_SpriteList = m_SpriteAtlas->GetSpriteList(name);
	m_CurrentSpriteKey = m_SpriteList.begin()->first;
	m_CurrentFrame = 1;
	m_TimeUntilNextFrame = m_TimePerFrame = 0.2f;
	// create one geometry + texture per sprite in the sprite list, and key this information for later use
	int spriteSwitcherIndex = 0;
	for (auto& sprite : m_SpriteList)
	{
		for (size_t i = 0; i < sprite.second.size(); i++)
		{
			std::string spriteName = sprite.second[i];
			std::cout << "Making Geo for sprite " << spriteName << " at switcherIndex " << spriteSwitcherIndex << std::endl;
			m_SpriteIdToIndexMap[spriteName] = spriteSwitcherIndex++;
			ImageRecPtr image = m_SpriteAtlas->GetImage(spriteName);
			SimpleTexturedMaterialRecPtr material = SimpleTexturedMaterial::create();
			material->setImage(image);
			GeometryRecPtr spriteGeo = makePlaneGeo(image->getWidth(), image->getHeight(), 1, 1);
			NodeRecPtr spriteGeoNode = Node::create();
			spriteGeoNode->setCore(spriteGeo);
			spriteGeo->setMaterial(material);
			m_SpriteSwitcher.node()->addChild(spriteGeoNode);
		}
	}
}

void Sprite::SetSprite(std::string key, int frame)
{
	std::string spriteName = m_SpriteAtlas->GetSpriteName(m_Name, key, frame);
	auto iter = m_SpriteIdToIndexMap.find(spriteName);
	if (iter != m_SpriteIdToIndexMap.end())
	{
		int spriteSwitcherIndex = iter->second;
		m_SpriteSwitcher->setChoice(spriteSwitcherIndex);
		m_CurrentFrame = frame;
	}
}

void Sprite::UpdateCurrentSprite(std::string key)
{
	m_CurrentSpriteKey = key;
}

void Sprite::SetTimePerFrame(float timePerFrame)
{
	m_TimePerFrame = timePerFrame;
}

Sprite::~Sprite()
{
}

void Sprite::Update()
{
	m_TimeUntilNextFrame -= Time::DeltaTime;
	if(m_TimeUntilNextFrame < 0)
	{
		int nextFrame = (++m_CurrentFrame % m_SpriteList[m_CurrentSpriteKey].size()) + 1;
		SetSprite(m_CurrentSpriteKey, nextFrame);
		m_TimeUntilNextFrame = m_TimePerFrame;
	}
}
