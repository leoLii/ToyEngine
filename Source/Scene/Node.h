#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <typeinfo>
#include <typeindex>

//class Component;

class Node {
public:
	Node();

	~Node();

	void setParent(Node*);

	const Node* getParent() const;

	void addChild(Node*);

	const std::vector<std::shared_ptr<Node>> getChildren() const;

	const uint32_t getID() const;

	virtual void update(float deltaTime);

	//void addComponent(Component*);

	//Component* getComponent() const;

	//void removeComponent(std::type_index);

	//bool hasComponent(Component*);

protected:
	uint32_t id;

	std::string name = "";

	Node* parent{ nullptr };

	std::vector<std::shared_ptr<Node>> children;

	//std::unordered_map<std::type_index, Component*> components;
};