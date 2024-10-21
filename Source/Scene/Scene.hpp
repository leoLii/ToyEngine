#pragma once

#include <vector>
#include <string>

#include "Common/Math.hpp"

class Node;
class Mesh;
class Camera;

class Scene {
public:
	Scene();
	~Scene();

	Node* getRootNode();

	Node* loadModel(std::string& path);

	std::vector<Mesh*> getMeshes() const;
	Camera* getCamera();
	void collectMeshes();
protected:
	Node* rootNode = nullptr;
	Node* cameraNode = nullptr;
	std::vector<Node*> nodes;
	Camera* camera;
	std::vector<Mesh*> meshes;

private:
	Vec3 extractTranslation(const Mat4& matrix) const;
	float distanceSquared(const Vec3& a, const Vec3& b) const;
};