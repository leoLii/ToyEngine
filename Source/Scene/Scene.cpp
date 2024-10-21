#include "Scene.hpp"

#include "Loader.hpp"

Scene::Scene() 
{
	rootNode = new Node("root");
	cameraNode = new Node{ "camera" };
	camera = new Camera{ CameraType::Perspect, Frustum{60.0f, 1920.0f, 1080.0f, 0.1f, 3000.0f} };
	cameraNode->addComponent(camera);
	rootNode->addChild(cameraNode);
}

Scene::~Scene()
{
	for (auto node : nodes) {
		delete node;
	}
	nodes.clear();
	meshes.clear();
}

Node* Scene::getRootNode()
{
	return this->rootNode;
}

Node* Scene::loadModel(std::string& path)
{
	AssimpLoader loader;
	return loader.loadModel(path);
}

void Scene::collectMeshes()
{
	if (rootNode) {
		// 使用 std::invoke 来递归调用 lambda
		auto collectMeshesLambda = [&](auto&& self, Node* node) -> void {
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
				std::invoke(self, self, child);  // 使用 std::invoke 递归调用 lambda
			}
			};

		// 调用 lambda
		collectMeshesLambda(collectMeshesLambda, rootNode);
	}

	// 收集完成后，对节点进行排序（根据相机距离，从远到近）
	std::sort(meshes.begin(), meshes.end(), [&](Mesh* a, Mesh* b) {
		Vec3 positionA = extractTranslation(a->getAttachNode()->getTransform());  // 提取节点 A 的世界坐标
		Vec3 positionB = extractTranslation(b->getAttachNode()->getTransform());  // 提取节点 B 的世界坐标
		
		Vec3 cameraPosition = extractTranslation(cameraNode->getTransform());

		float distanceA = distanceSquared(positionA, cameraPosition);  // 计算 A 到相机的距离
		float distanceB = distanceSquared(positionB, cameraPosition);  // 计算 B 到相机的距离

		return distanceA < distanceB;  // 从远到近排序
		});
}

std::vector<Vertex>& Scene::getVertices()
{
	std::vector<Vertex> allVertexData;
	for (auto mesh : meshes) {
		auto vertexData = mesh->assembleVertexData();
		allVertexData.insert(allVertexData.end(), vertexData.begin(), vertexData.end());
	}
	return allVertexData;
}

std::vector<uint32_t>& Scene::getIndices()
{
	std::vector<uint32_t> allIndexData;
	for (auto mesh : meshes) {
		auto indexData = mesh->getIndices();
		allIndexData.insert(allIndexData.end(), indexData.begin(), indexData.end());
	}
	return allIndexData;
}

std::vector<Mat4>& Scene::getUniforms()
{
	std::vector<Mat4> allTransform;
	for (auto mesh : meshes) {
		auto transform = mesh->getAttachNode()->getTransform();
		allTransform.push_back(transform);
	}
	return allTransform;
}

// 从矩阵中提取平移分量（假设列主序矩阵）
Vec3 Scene::extractTranslation(const Mat4& matrix) const {
	return Vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
}

// 计算两个向量的平方距离
float Scene::distanceSquared(const Vec3& a, const Vec3& b) const {
	Vec3 diff = a - b;
	return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
}

std::vector<Mesh*> Scene::getMeshes() const
{
	return this->meshes;
}

Camera* Scene::getCamera()
{
	return this->camera;
}
