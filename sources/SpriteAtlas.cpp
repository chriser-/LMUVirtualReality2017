#include "SpriteAtlas.h"
#include <irrXML/irrXML.h>
#include "boost/gil/extension/io/png_io.hpp"
#include <boost/algorithm/string.hpp>

SubImageInfo::SubImageInfo(std::string Name, bool MirrorHorizontal, bool MirrorVertical, int OffsetX, int OffsetY, int Width, int Height) :
	Name(Name), MirrorHorizontal(MirrorHorizontal),
	MirrorVertical(MirrorVertical),
	OffsetX(OffsetX),
	OffsetY(OffsetY),
	Width(Width),
	Height(Height)
{
}

SpriteAtlas::SpriteAtlas(std::string name)
{
	m_Name = name;
	ImageRecPtr image = Image::create();
	image->read(("sprites/" + m_Name + "/spritesheet.png").c_str());
	m_SpriteMap["full"] = image;
	std::cout << "full image=" << image->getWidth() << "x" << image->getHeight() << std::endl;
	for (auto element : LoadSubImageInfo(m_Name))
	{
		ImageRecPtr subImage = Image::create();
		image->subImage(element.OffsetX, image->getHeight() - element.OffsetY - element.Height, 0, element.Width, element.Height, image->getDepth(), subImage);
		subImage->mirror(element.MirrorHorizontal, element.MirrorVertical, false, subImage);
		m_SpriteMap[element.Name] = subImage;
	}
	m_SpriteListMap = LoadSpriteList(name);
}

ImageTransitPtr SpriteAtlas::GetImage(std::string name)
{
	const auto iter = m_SpriteMap.find(name);
	if (iter != m_SpriteMap.end())
	{
		return ImageTransitPtr(iter->second);
	}

	return ImageTransitPtr(nullptr);
}

std::unordered_map<std::string, std::vector<std::string>> SpriteAtlas::GetSpriteList(std::string name)
{
	return m_SpriteListMap[name];
}

std::string SpriteAtlas::GetSpriteName(std::string sprite, std::string key, int frame)
{
	std::string spriteName;
	const auto iter1 = m_SpriteListMap.find(sprite);
	if(iter1 != m_SpriteListMap.end())
	{
		const auto iter2 = m_SpriteListMap[sprite].find(key);
		if(iter2 != m_SpriteListMap[sprite].end())
		{
			if(frame <= m_SpriteListMap[sprite][key].size())
			{
				spriteName = m_SpriteListMap[sprite][key][frame - 1];
			}
		}
	}
	return spriteName;
}

SpriteAtlas::~SpriteAtlas()
{
}

using namespace irr; // irrXML is located in the namespace irr::io
using namespace io;

std::vector<SubImageInfo> SpriteAtlas::LoadSubImageInfo(std::string name)
{
	std::string filename = "sprites/" + name + "/spritesheet.xml";
	std::cout << "Loading SubImageInfo config file from " << filename << std::endl;
	IrrXMLReader* xml = createIrrXMLReader(filename.c_str());

	std::vector<SubImageInfo> subImageInfos;

	while (xml && xml->read())
	{
		switch (xml->getNodeType())
		{
		case EXN_ELEMENT:
		{
			if (!strcmp("subtexture", xml->getNodeName()))
			{
				subImageInfos.push_back(
					SubImageInfo(
						xml->getAttributeValue("name"),
						strcmp("true", xml->getAttributeValue("MirrorHorizontal")) == 0,
						strcmp("true", xml->getAttributeValue("MirrorVertical")) == 0,
						xml->getAttributeValueAsInt("x"),
						xml->getAttributeValueAsInt("y"),
						xml->getAttributeValueAsInt("width"),
						xml->getAttributeValueAsInt("height")
					)
				);
			}
		}
		break;
		default: ;
		}
	}

	// delete the xml parser after usage
	delete xml;
	std::cout << "Creating Sprite Atlas from " << filename << " with " << subImageInfos.size() << " sprites" << std::endl;
	return subImageInfos;
}

std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> SpriteAtlas::LoadSpriteList(std::string name)
{
	std::string fileName = "sprites/" + name + "/spritelist.xml";
	std::cout << "Loading SpriteList from " << fileName << std::endl;
	IrrXMLReader* xml = createIrrXMLReader(fileName.c_str());
	
	std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> spriteListMap;
	std::string currentSprite;
	std::string currentKey;
	int currentFrame = 1;
	std::string spriteName;

	while (xml && xml->read())
	{
		switch (xml->getNodeType())
		{
		case EXN_ELEMENT:
			if (!strcmp("sprite", xml->getNodeName()))
			{
				currentSprite = xml->getAttributeValue("name");
			}
			else if (!strcmp("item", xml->getNodeName()))
			{
				currentKey = xml->getAttributeValue("key");
				currentFrame = xml->getAttributeValueAsInt("frame");
			}
			break;
		case EXN_TEXT:
			spriteName = xml->getNodeData();
			boost::trim(spriteName);
			if (spriteName.size() > 0) {
				std::cout << "" << name << ": [" << currentSprite << "][" << currentKey << "][" << (currentFrame - 1) << "] = " << spriteName << " (" << spriteName.size() << ")" << std::endl;
				spriteListMap[currentSprite][currentKey].insert(spriteListMap[currentSprite][currentKey].begin() + currentFrame - 1, spriteName);
			}
			break;
		default:
			break;
		}
	}

	// delete the xml parser after usage
	delete xml;
	
	return spriteListMap;
}
