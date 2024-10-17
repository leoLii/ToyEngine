#include "Component.hpp"

Component::Component(const std::string& name)
	:name{ name }
{
}

const std::string& Component::getName() const
{
	return name;
}
