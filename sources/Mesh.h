#pragma once
#include "Component.h"

class Mesh : public Component
{
public:
	Mesh(OSG::NodeRecPtr parentNode, std::string name);
};
