#pragma once

#include <vector>
#include <string>

class Node;
class Mesh;
class Camera;

class Scene {
public:
	Scene();
	~Scene();

	Node* getRootNode();

	void loadModel(std::string& path);

	std::vector<Mesh*> getMeshes() const;
	Camera* getCamera();

protected:
	Node* rootNode = nullptr;
	std::vector<Node*> nodes;
	Camera* camera;
	std::vector<Mesh*> meshes;

private:
	void collectMeshes();
};