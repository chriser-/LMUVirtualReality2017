#include "Component.h"
#include <OSGTransitPtr.h>

Component::~Component()
{
}

void Component::Update()
{
}

NodeTransitPtr Component::GetNode() const
{
	return NodeTransitPtr(m_Node);
}

size_t Component::hash_value(const Component& a)
{
	const size_t hash = reinterpret_cast<size_t>(&a);
	return hash;
}
