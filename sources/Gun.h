#pragma once
#include "GameObject.h"
#include <OSGIntersectAction.h>

class Gun : public GameObject
{
private:
	GameObject* m_barrelExitPosition;
	OSG::NodeRecPtr crosshair_display_plane_;
	OSG::Line CalculateRay();
	OSG::NodeRecPtr m_crosshairDisplayPlane;
	GameObject* m_crosshair;
	int m_remainingShots;
	OSG::ComponentTransformNodeRefPtr m_lineStart, m_lineEnd;
public:
	Gun();
	void Update() override;
	void Shoot();
	void SetRemainingShots(int shots);
	GameObject* GetBarrelExitGameObject();
};
