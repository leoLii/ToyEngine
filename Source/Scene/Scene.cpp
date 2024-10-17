#include "Scene.hpp"

#include "Mesh.hpp"
#include "Loader.hpp"
#include "Node.hpp"
#include "Scene/Components/Camera.hpp"

Scene::Scene() 
{
	rootNode = new Node("root");
	Node* cameraNode = new Node{ "camera" };
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

void Scene::loadModel(std::string& path)
{
	AssimpLoader loader;
	rootNode->addChild(loader.loadModel(path));
	collectMeshes();
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
}

std::vector<Mesh*> Scene::getMeshes() const
{
	return this->meshes;
}

Camera* Scene::getCamera()
{
	return this->camera;
}
