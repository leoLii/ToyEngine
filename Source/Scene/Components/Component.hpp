#pragma once

#include <string>
#include <typeindex>

class Node;

class Component {
public:
	Component() = default;
	Component(const std::string&);
	Component(Component&& other) = default;
	virtual ~Component() = default;

	const std::string& getName() const;
	virtual std::type_index getType() = 0;

	virtual void update(float deltaTime, uint32_t frameIndex) = 0;
	virtual void lateUpdate() = 0;

	virtual Node* getAttachNode() const;

	virtual void setAttachNode(Node*);

protected:
	std::string name;

	Node* attachNode;
};