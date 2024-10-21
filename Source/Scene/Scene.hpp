#pragma once

#include <vector>
#include <string>

#include "Common/Math.hpp"
#include "Mesh.hpp"
#include "Node.hpp"
#include "Scene/Components/Camera.hpp"

class Scene {
public:
	Scene();
	~Scene();

	Node* getRootNode();

	Node* loadModel(std::string& path);

	std::vector<Mesh*> getMeshes() const;
	Camera* getCamera();
	void collectMeshes();

	std::vector<Vertex>& getVertices();
	std::vector<uint32_t>& getIndices();
	std::vector<Mat4>& getUniforms();
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