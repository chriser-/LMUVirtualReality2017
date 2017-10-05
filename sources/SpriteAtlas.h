#pragma once
#include "GameObject.h"
#include <OpenSG/OSGSimpleTexturedMaterial.h>
#include <OpenSG/OSGImage.h>
#include <unordered_map>

struct SubImageInfo {
	std::string Name;
	bool MirrorHorizontal;
	bool MirrorVertical;
	int  OffsetX;
	int  OffsetY;
	int  Width;
	int  Height;

	SubImageInfo(
		std::string Name,
		bool MirrorHorizontal,
		bool MirrorVertical,
		int  OffsetX,
		int  OffsetY,
		int  Width,
		int  Height);
};

class SpriteAtlas
{
private:
	std::unordered_map<std::string, OSG::ImageRecPtr> m_SpriteMap;
	std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> m_SpriteListMap;
	std::string m_Name;
public:
	SpriteAtlas(std::string name);
	~SpriteAtlas();
	static std::vector<SubImageInfo> LoadSubImageInfo(std::string name);
	OSG::ImageTransitPtr GetImage(std::string name);
	std::unordered_map<std::string, std::vector<std::string>> GetSpriteList(std::string name);
	static std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> LoadSpriteList(std::string name);
	std::string GetSpriteName(std::string sprite, std::string key, int frame);
};
