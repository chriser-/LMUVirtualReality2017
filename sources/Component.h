#pragma once
#include <boost/unordered/unordered_set.hpp>
#include <OSGConfig.h>
#include <OSGNode.h>

class Component
{
protected:
	OSG::NodeRecPtr m_Node;
public:
	virtual ~Component();
	virtual void Update();
	OSG::NodeTransitPtr GetNode() const;
	static size_t hash_value(const Component& a);
};
