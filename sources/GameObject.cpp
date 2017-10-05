#include "GameObject.h"
#include "Game.h"
#include <OSGSimpleGeometry.h>
#include <OSGComponentTransform.h>
#include <OSGGroup.h>
#include <OSGNameAttachment.h>

GameObject::~GameObject()
{
	Game::Instance()->RemoveBehavior(this);
	for (auto && child : m_Children)
	{
		delete child;
	}
	for (auto && component : m_Components)
	{
		delete component;
	}
}

void GameObject::ctor(std::string name, OSG::ComponentTransformNodeRefPtr root)
{
	m_Name = name;
	setName(root.node(), name);
	m_Transform = root;
	m_Group = OSG::GroupNodeRefPtr::create();
	m_Transform.node()->addChild(m_Group);
	Game::Instance()->AddBehavior(this);
}

GameObject::GameObject(std::string name, OSG::ComponentTransformNodeRefPtr rootNode)
{
	ctor(name, rootNode);
}

GameObject::GameObject(std::string name)
{
        ctor(name, OSG::ComponentTransformNodeRefPtr::create());
}
GameObject::GameObject()
{
        ctor("GameObject", OSG::ComponentTransformNodeRefPtr::create());
}

void GameObject::Update()
{
	for (auto component : m_Components)
	{
		component->Update();
	}
}

void GameObject::Translate(OSG::Vec3f by) const
{
	GetTransform()->setTranslation(GetTransform()->getTranslation() + by);
}

void GameObject::AddChild(GameObject* other)
{
	m_Children.insert(other);
	m_Group.node()->addChild(other->GetTransform().node());
}

void GameObject::AddComponent(Component* component)
{
	
	if(component->GetNode().get() != nullptr)
	{
		m_Group.node()->addChild(component->GetNode());
	}
	m_Components.insert(component);

}

void GameObject::RemoveComponent(Component* component)
{
	if (component->GetNode().get() != nullptr)
	{
		m_Group.node()->subChild(component->GetNode().get());
	}
	m_Components.erase(component);
}

void GameObject::OnHit()
{
}

size_t GameObject::hash_value(const GameObject& a)
{
	const size_t hash = reinterpret_cast<size_t>(&a);
	return hash;
}

std::string GameObject::GetName() const
{
	return m_Name;
}

OSG::ComponentTransformNodeRefPtr GameObject::GetTransform() const
{
	return m_Transform;
}
