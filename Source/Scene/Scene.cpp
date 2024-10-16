#include "Scene.hpp"

#include "Mesh.hpp"
#include "Loader.hpp"
#include "Node.hpp"

Scene::Scene() 
{
	rootNode = new Node("root");
}

Scene::~Scene()
{
	for (auto node : nodes) {
		delete node;
	}
	nodes.clear();
	meshes.clear();
}

void Scene::loadModel(std::string& path)
{
	AssimpLoader loader;
	rootNode->addChild(loader.loadModel(path));
}

void Scene::collectMeshes()
{
	if (rootNode) {
		collectMeshesFromNode(rootNode);
	}
}

void Scene::collectMeshesFromNode(Node* node)
{
	if (node == nullptr) {
		return;
	}

	nodes.push_back(node);
	Mesh* nodeMesh = node->getMesh();
	if (nodeMesh) {
		this->meshes.push_back(nodeMesh);
	}
	
	// 递归遍历子节点
	const std::vector<Node*>& children = node->getChildren();
	for (Node* child : children) {
		collectMeshesFromNode(child);
	}
}

std::vector<Mesh*> Scene::getMeshes() const
{
	return this->meshes;
}
