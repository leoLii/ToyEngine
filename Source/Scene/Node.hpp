#pragma once

#include "Common/Math.hpp"
#include "Scene/Components/Transform.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <typeinfo>
#include <typeindex>

class Mesh;
class Component;

class Node {
public:
	Node();

	Node(const char* name);

	~Node();

	void update(float, uint32_t);

	void lateUpdate();

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

	Transform& getTransform();

	void setTransform(Mat4);

	void setTranslate(Vec3);
	void setRotate(float, Vec3);
	void setScale(Vec3);

	//virtual void update(float deltaTime) = 0;

	void addComponent(Component*);

	Component* getComponent() const;

	void removeComponent(std::type_index);

	bool hasComponent(std::type_index);

protected:
	uint32_t id;
	std::string name = "";
	bool isActive = true;

	Node* parent{ nullptr };

	std::vector<Node*> children;

	Transform transform;

	Mesh* mesh = nullptr;

	std::unordered_map<std::type_index, Component*> components;
};