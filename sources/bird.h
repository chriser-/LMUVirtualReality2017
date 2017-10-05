#pragma once

#include <OSGComponentTransformBase.h>
#include "GameObject.h"
#include "Sprite.h"

class Bird : public GameObject
{
private:
	OSG::Vec3f m_direction;
	int m_speed;
	bool m_hit;
	bool m_dead;
	float m_hitSpriteTimer;
	float m_nextDirectionTimer;
	static const int START_VELOCITY_COUNT;
	static const OSG::Vec3f START_VELOCITIES[];
	static const std::string VELOCITY_TO_SPRITE[3][3];
	Sprite* m_BirdSprite;
	void UpdateDirectionSprite() const;
public:
	Bird(std::string name, std::string spriteName);
	void Update() override;
	void SetSpeed(float speed);
	void SetSprite(Sprite* sprite);
	void OnHit() override;
	~Bird() override;
};
