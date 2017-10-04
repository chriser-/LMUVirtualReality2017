#include "Sprite.h"
#include <OSGSimpleTexturedMaterial.h>
#include <OSGImage.h>
#include <OSGSwitch.h>
#include "Game.h"
#include <OSGGeometry.h>
#include <OSGSimpleGeometry.h>
#include "Time.h"

Sprite::Sprite(NodeRecPtr parentNode, std::string name, std::string spriteAtlas, int sortKey)
{
	m_Name = name;
	m_SpriteAtlas = Game::Instance()->GetSpriteAtlas(spriteAtlas);
	m_SpriteSwitcher = OSG::SwitchNodeRefPtr::create();
	m_Node = m_SpriteSwitcher;
	std::cout << "Adding sprite switcher (" << m_SpriteSwitcher.node() << " to node " << parentNode << std::endl;
	parentNode->addChild(m_SpriteSwitcher);
	m_SpriteList = m_SpriteAtlas->GetSpriteList(name);
	m_CurrentSpriteKey = m_SpriteList.begin()->first;
	m_CurrentFrame = 1;
	m_forceSpriteUpdate = false;
	m_TimeUntilNextFrame = m_TimePerFrame = 0.2f;
	// create one geometry + texture per sprite in the sprite list, and key this information for later use
	int spriteSwitcherIndex = 0;
	for (auto& sprite : m_SpriteList)
	{
		for (size_t i = 0; i < sprite.second.size(); i++)
		{
			const std::string spriteName = sprite.second[i];
			std::cout << "Making Geo for sprite " << spriteName << " at switcherIndex " << spriteSwitcherIndex << std::endl;
			m_SpriteIdToIndexMap[spriteName] = spriteSwitcherIndex++;
                        const OSG::ImageRecPtr image = m_SpriteAtlas->GetImage(spriteName);
                        OSG::SimpleTexturedMaterialRecPtr material = OSG::SimpleTexturedMaterial::create();
			material->setImage(image);
			material->setSortKey(sortKey);
                        OSG::GeometryRecPtr spriteGeo = OSG::makePlaneGeo(image->getWidth()+1, image->getHeight()+1, 1, 1);
                        OSG::NodeRecPtr spriteGeoNode = OSG::Node::create();
			spriteGeoNode->setCore(spriteGeo);
			spriteGeo->setMaterial(material);
			m_SpriteSwitcher.node()->addChild(spriteGeoNode);
		}
	}
	SetSprite(m_CurrentSpriteKey, m_CurrentFrame);
}

void Sprite::SetSprite(std::string key, int frame)
{
	const std::string spriteName = m_SpriteAtlas->GetSpriteName(m_Name, key, frame);
	const auto iter = m_SpriteIdToIndexMap.find(spriteName);
	if (iter != m_SpriteIdToIndexMap.end())
	{
		const int spriteSwitcherIndex = iter->second;
		m_SpriteSwitcher->setChoice(spriteSwitcherIndex);
		m_CurrentFrame = frame;
	}
}

void Sprite::UpdateCurrentSprite(std::string key, int frame)
{
	m_CurrentSpriteKey = key;
	if(frame > 0)
	{
		m_forceSpriteUpdate = true;
		m_CurrentFrame = frame;
	}
}

void Sprite::SetTimePerFrame(float timePerFrame)
{
	m_TimePerFrame = timePerFrame;
}

Vec2f Sprite::GetDimensions() const
{
        const OSG::GeometryRecPtr currentActiveGeometry = dynamic_cast<OSG::Geometry*>(m_SpriteSwitcher.node()->getChild(m_SpriteSwitcher->getChoice())->getCore());
	if (currentActiveGeometry != nullptr)
	{
                const OSG::SimpleTexturedMaterialRecPtr material = dynamic_cast<OSG::SimpleTexturedMaterial*>(currentActiveGeometry->getMaterial());
                return OSG::Vec2f(material->getImage()->getWidth(), material->getImage()->getHeight());
	}
        return OSG::Vec2f(0, 0);
}

Sprite::~Sprite()
{
}

void Sprite::Update()
{
	m_TimeUntilNextFrame -= MyTime::DeltaTime;
	if(m_TimeUntilNextFrame < 0 || m_forceSpriteUpdate)
	{
		const int nextFrame = ((m_CurrentFrame+1) % m_SpriteList[m_CurrentSpriteKey].size()) + 1;
		if (m_CurrentFrame != nextFrame || m_forceSpriteUpdate)
		{
			m_CurrentFrame = nextFrame;
			SetSprite(m_CurrentSpriteKey, m_CurrentFrame);
			m_TimeUntilNextFrame = m_TimePerFrame;
		}
	}
}
