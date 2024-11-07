#include "Node.hpp"
#include "Mesh.hpp"

#include "Components/Component.hpp"

#include <random>

Node::Node() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
	id = dist(gen);
}

Node::Node(const char* name)
	:name{ name }
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
	id = dist(gen);
}

Node::~Node() {
	delete mesh;
	for (auto component : components) {
		delete component.second;
	}
	components.clear();
}

void Node::update(float deltaTime, uint32_t frameIndex)
{
	transform.update(deltaTime, frameIndex);
	for (auto component : components) {
		component.second->update(deltaTime, frameIndex);
	}
}

void Node::lateUpdate()
{
	for (auto component : components) {
		component.second->lateUpdate();
	}
}

void Node::setParent(Node* parent) {
	this->parent = parent;
}

const Node* Node::getParent() const
{
	return this->parent;
}

void Node::addChild(Node* child)
{
	child->setParent(this);
	this->children.push_back(child);
}

const std::vector<Node*>& Node::getChildren() const
{
	return children;
}

bool Node::hasChild() const
{
	return !this->children.empty();
}

uint32_t Node::getID() const
{
	return id;
}

void Node::setName(std::string name)
{
	this->name = name;
}

std::string Node::getName()
{
	return this->name;
}

void Node::setMesh(Mesh* mesh)
{
	this->mesh = mesh;
	mesh->setAttachNode(this);
}

Mesh* Node::getMesh() 
{
	return this->mesh;
}

Transform& Node::getTransform() 
{
	return this->transform;
}

void Node::setTransform(Mat4 transform)
{
	this->transform.setTransform(transform);
	for (auto child : children) {
		child->setTransform(transform);
	}
}

void Node::setTranslate(Vec3 translation)
{
	this->transform.setTranslate(translation);
	for (auto child : children) {
		child->setTranslate(translation);
	}
}

void Node::setRotate(float angle, Vec3 axis)
{
	this->transform.setRotate(angle, axis);
	for (auto child : children) {
		child->setRotate(angle, axis);
	}
}

void Node::setScale(Vec3 scale)
{
	this->transform.setScale(scale);
	for (auto child : children) {
		child->setScale(scale);
	}
}

void Node::addComponent(Component* component)
{
	this->components[typeid(component)] = component;
	component->setAttachNode(this);
}

