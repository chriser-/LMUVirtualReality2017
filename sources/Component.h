#pragma once
#include <boost/unordered/unordered_set.hpp>
#include <OSGConfig.h>
#include <OSGNode.h>

OSG_USING_NAMESPACE

class Component
{
protected:
	NodeRecPtr m_Node;
public:
	virtual ~Component();
	virtual void Update();
	NodeTransitPtr GetNode() const;
	static size_t hash_value(const Component& a);
};
