#include "Node.h"

#include <random>

Node::Node() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
	id = dist(gen);
}

Node::~Node() {

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
	this->children.push_back(std::make_shared<Node>(child));
}

const std::vector<std::shared_ptr<Node>> Node::getChildren() const
{
	return children;
}

const uint32_t Node::getID() const
{
	return id;
}

