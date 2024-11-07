#include "Component.hpp"

Component::Component(const std::string& name)
	:name{ name }
{
}

const std::string& Component::getName() const
{
	return name;
}

Node* Component::getAttachNode() const
{
	return attachNode;
}

void Component::setAttachNode(Node* node)
{
	attachNode = node;
}
