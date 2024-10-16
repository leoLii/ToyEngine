#pragma once

#include "Common/Math.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <typeinfo>
#include <typeindex>

class Mesh;

class Node {
public:
	Node();

	Node(const char* name);

	~Node();

	void setParent(Node*);

	const Node* getParent() const;

	void addChild(Node*);

	const std::vector<Node*>& getChildren() const;

	bool hasChild() const;

	uint32_t getID() const;

	void setName(std::string);

	std::string getName();

	void setMesh(Mesh*);

	Mesh* getMesh();

	

	//virtual void update(float deltaTime) = 0;

	//void addComponent(Component*);

	//Component* getComponent() const;

	//void removeComponent(std::type_index);

	//bool hasComponent(Component*);

protected:
	uint32_t id;
	std::string name = "";
	bool isActive = true;

	Node* parent{ nullptr };

	std::vector<Node*> children;

	Mat4 transform{ 1.0 };

	Mesh* mesh = nullptr;

	//std::unordered_map<std::type_index, Component*> components;
};