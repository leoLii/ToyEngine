#include "Node.hpp"
#include "Mesh.hpp"

#include "Component.hpp"

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
	child->setTransform(this->getTransform());
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

Mesh* Node::getMesh() {
	return this->mesh;
}

void Node::setTransform(Mat4 matrix)
{
	this->transform *= matrix;
	for (auto child : children)
	{
		child->setTransform(matrix);
	}
}

Mat4 Node::getTransform()
{
	return transform;
}

void Node::addComponent(Component* component)
{
	this->components[typeid(component)] = component;
}

