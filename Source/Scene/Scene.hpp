#pragma once

#include "Common/Math.hpp"
#include "Mesh.hpp"
#include "Node.hpp"
#include "Scene/Components/Camera.hpp"

#include <vector>
#include <string>
#include <map>
#include <atomic>

class Material;

class Scene {
public:
	Scene();
	~Scene();

	void update(uint32_t);

	bool getIsReady() {
		return isReady.load();
	}

	Node* getRootNode();

	Node* loadModel(std::string& path);

	std::vector<Mesh*> getMeshes() const;
	Camera* getCamera() const;
	void collectMeshes();

	const std::vector<Vertex> getVertices() const;
	const std::vector<uint32_t> getIndices() const;
	const std::vector<Mat4> getTransforms() const;
	const std::vector<Mat4> getPrevTransforms() const;

	const uint32_t getMeshCount() const {
		return meshes.size();
	}

	std::vector<uint32_t> vertexOffsets;
	std::vector<uint32_t> indexOffsets;
	std::vector<uint32_t> bufferOffsets;

	void addMaterial(uint32_t, Material*);
	const std::map<uint32_t, Material*>& getMaterials() const;

protected:
	Node* rootNode = nullptr;
	Node* cameraNode = nullptr;
	std::vector<Node*> nodes;
	Camera* camera;
	std::vector<Mesh*> meshes;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Mat4> transforms;
	std::vector<Mat4> prevTransforms;

	std::map<uint32_t, Material*> materials;

	std::atomic<bool> isReady = false;

private:
	Vec3 extractTranslation(const Mat4& matrix) const;
	float distanceSquared(const Vec3& a, const Vec3& b) const;
};