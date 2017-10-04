#pragma once

#include <OSGComponentTransformBase.h>
#include "GameObject.h"
#include "Sprite.h"

class Bird : public GameObject
{
private:
	Vec3f m_direction;
	int m_speed;
	static const int START_VELOCITY_COUNT;
	static const Vec3f START_VELOCITIES[];
	static const std::string VELOCITY_TO_SPRITE[3][3];
	Sprite* m_BirdSprite;
public:
	Bird(std::string name, std::string spriteName);
	void Update() override;
	void SetSpeed(float speed);
	void SetSprite(Sprite* sprite);
	void OnHit() override;
	~Bird() override;
};
