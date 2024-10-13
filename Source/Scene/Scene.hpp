#pragma once

#include <vector>
#include <string>

class Node;
class Mesh;

class Scene {
public:
	Scene();
	~Scene();

	void loadModel(std::string& path);

	void collectMeshes();

	void collectMeshesFromNode(Node*);

	std::vector<Mesh*> getMeshes() const;

protected:
	Node* rootNode = nullptr;
	std::vector<Node*> nodes;

	std::vector<Mesh*> meshes;
};