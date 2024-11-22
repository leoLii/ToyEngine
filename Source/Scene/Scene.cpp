#include "Scene.hpp"

#include "Loader.hpp"

Scene::Scene() 
{
	rootNode = new Node("root");
	cameraNode = new Node{ "camera" };
	camera = new Camera{ CameraType::Perspect, Frustum{90.0f, 1920.0f, 1080.0f, 0.1f, 3000.0f} };
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

void Scene::update(uint32_t frameIndex)
{
	float deltaTime = 1.0f / 60.0f;
	float rotationSpeed = 1.0f;
	float movementSpeed = 0.0001f;
	float movementRange = 1.0f;

	static int direction = 1;
	static int count = 0;
	prevTransforms.assign(transforms.begin(), transforms.end());

	/*uniforms.clear();

	if (count == 5000) {
		count = 0;
		direction = -direction;
	}

	for (auto mesh : meshes) {
		Mat4 transform{ 1.0 };
		auto node = mesh->getAttachNode();

		float angle = rotationSpeed * deltaTime;

		Mat4 rotationMatrix = glm::rotate(Mat4(1.0f), glm::radians(angle), Vec3(0.0f, -1.0f, 0.0f));

		node->setTransform(rotationMatrix);
		float positionX = direction*movementSpeed;
		node->setTranslate(Vec3(positionX, 0.0, 0.0));

		uniforms.push_back(node->getTransform().getCurrMatrix());
	}*/

	for (auto node : nodes) {
		node->update(0.0, frameIndex);
	}

	for(int i = 0; i < meshes.size(); i++){
		auto transform = meshes[i]->getAttachNode()->getTransform().getCurrMatrix();
		transforms[i] = transform;
	}

	count++;
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
		// ʹ�� std::invoke ���ݹ���� lambda
		auto collectMeshesLambda = [&](auto&& self, Node* node) -> void {
			if (node == nullptr) {
				return;
			}

			nodes.push_back(node);
			Mesh* nodeMesh = node->getMesh();
			if (nodeMesh) {
				this->meshes.push_back(nodeMesh);
			}

			// �ݹ�����ӽڵ�
			const std::vector<Node*>& children = node->getChildren();
			for (Node* child : children) {
				std::invoke(self, self, child);  // ʹ�� std::invoke �ݹ���� lambda
			}
			};

		// ���� lambda
		collectMeshesLambda(collectMeshesLambda, rootNode);
	}

	// �ռ���ɺ󣬶Խڵ�������򣨸���������룬��Զ������
	std::sort(meshes.begin(), meshes.end(), [&](Mesh* a, Mesh* b) {
		Vec3 positionA = extractTranslation(a->getAttachNode()->getTransform().getCurrMatrix());  // ��ȡ�ڵ� A ����������
		Vec3 positionB = extractTranslation(b->getAttachNode()->getTransform().getCurrMatrix());  // ��ȡ�ڵ� B ����������
		
		Vec3 cameraPosition = extractTranslation(cameraNode->getTransform().getCurrMatrix());

		float distanceA = distanceSquared(positionA, cameraPosition);  // ���� A ������ľ���
		float distanceB = distanceSquared(positionB, cameraPosition);  // ���� B ������ľ���

		return distanceA < distanceB;  // ��Զ��������
		});

	for (auto mesh : meshes) {
		vertexOffsets.push_back(vertices.size());
		auto vertexData = mesh->assembleVertexData();
		vertices.insert(vertices.end(), vertexData.begin(), vertexData.end());

		indexOffsets.push_back(indices.size());
		auto indexData = mesh->getIndices();
		indices.insert(indices.end(), indexData.begin(), indexData.end());

		bufferOffsets.push_back(transforms.size() * sizeof(Mat4) * 2);
		auto transform = mesh->getAttachNode()->getTransform().getCurrMatrix();
		transforms.push_back(transform);
	}

	prevTransforms.assign(transforms.begin(), transforms.end());
}

const std::vector<Vertex> Scene::getVertices() const
{
	return vertices;
}

const std::vector<uint32_t> Scene::getIndices() const
{
	return indices;
}

const std::vector<Mat4> Scene::getTransforms() const
{
	return transforms;
}

const std::vector<Mat4> Scene::getPrevTransforms() const
{
	return prevTransforms;
}

// �Ӿ�������ȡƽ�Ʒ������������������
Vec3 Scene::extractTranslation(const Mat4& matrix) const {
	return Vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
}

// ��������������ƽ������
float Scene::distanceSquared(const Vec3& a, const Vec3& b) const {
	Vec3 diff = a - b;
	return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
}

std::vector<Mesh*> Scene::getMeshes() const
{
	return this->meshes;
}

Camera* Scene::getCamera() const
{
	return this->camera;
}
