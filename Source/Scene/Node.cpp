#include "Node.hpp"
#include "Mesh.hpp"

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
}

Mesh* Node::getMesh() {
	return this->mesh;
}

