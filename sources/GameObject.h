#pragma once
#include <cstdint>
#include <OSGComponentTransform.h>
#include <OSGComponentTransformBase.h>
#include <boost/unordered/unordered_set.hpp>
#include <OSGGeometry.h>
#include <OSGSwitchMaterialBase.h>
#include <OSGSimpleTexturedMaterialBase.h>
#include "Component.h"

class SpriteAtlas;

class GameObject
{
protected:
	OSG::ComponentTransformNodeRefPtr m_Transform;
	OSG::GroupNodeRefPtr m_Group;
	boost::unordered_set<GameObject*> m_Children;
	boost::unordered_set<Component*> m_Components;
	std::string m_Name;
	void ctor(std::string name, OSG::ComponentTransformNodeRefPtr root);
public:
	virtual ~GameObject();
	GameObject(std::string name, OSG::ComponentTransformNodeRefPtr root);
	GameObject(std::string name);
	GameObject();
	virtual void Update();
	void Translate(OSG::Vec3f by) const;
	void AddChild(GameObject* other);
	void AddComponent(Component* component);
	void RemoveComponent(Component* component);
	virtual void OnHit();
	static size_t hash_value(const GameObject& a);
	OSG::ComponentTransformNodeRefPtr GetTransform() const;
	std::string GetName() const;
};
