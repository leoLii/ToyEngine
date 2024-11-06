#pragma once

#include <vector>
#include <string>
#include "Common/Math.hpp"

#include "Node.hpp"

struct AABB {
	Vec3 min; // 最小点
	Vec3 max; // 最大点

	AABB() : min(Vec3(0.0)), max(Vec3(0.0)) {}
	AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}

	// 合并两个 AABB
	AABB merge(const AABB& other) const {
		return AABB(glm::min(min, other.min), glm::max(max, other.max));
	}

	// 检查 AABB 是否与另一个 AABB 相交
	bool intersect(const AABB& other) const {
		return (max.x >= other.min.x && min.x <= other.max.x) &&
			(max.y >= other.min.y && min.y <= other.max.y) &&
			(max.z >= other.min.z && min.z <= other.max.z);
	}
};

struct Vertex {
	Vec3 position;
	Vec2 texcoord;
	Vec3 normal;
	Vec3 tangent;
	//Vec3 bitangent;
};

class Mesh {
public:
	Mesh() = default;
	Mesh(std::string& name);

	~Mesh() = default;

	std::string getName() 
	{
		return this->name;
	}

	void setVertices(std::vector<Vec3>&&);
	void setNormals(std::vector<Vec3>&&);
	void setTangents(std::vector<Vec3>&&);
	void setBitangents(std::vector<Vec3>&&);
	void setTexcoords(std::vector<Vec2>&&);
	void setIndices(std::vector<uint32_t>&&);

	void updateAABB();

	std::vector<Vec3>& getVertices() {
		return vertices;
	}

	std::vector<Vec3>& getNormals() {
		return normals;
	}

	std::vector<Vec3>& getTangents() {
		return tangents;
	}

	std::vector<Vec3>& getBitangents() {
		return bitangents;
	}

	std::vector<Vec2>& getTexcoords() {
		return texcoords;
	}

	std::vector<uint32_t>& getIndices() {
		return indices;
	}

	std::vector<Vertex> assembleVertexData() const {
		assert(vertices.size() == normals.size());  // 确保顶点数据和法线数据的大小一致
		assert(vertices.size() == tangents.size()); // 确保切线数据和顶点数据的大小一致
		//assert(vertices.size() == bitangents.size()); // 确保位切线和顶点数据大小一致
		assert(vertices.size() == texcoords.size());  // 确保纹理坐标和顶点数据大小一致

		std::vector<Vertex> vertexData;
		vertexData.reserve(vertices.size());  // 预先分配空间以避免不必要的动态分配

		for (size_t i = 0; i < vertices.size(); ++i) {
			Vertex vertex;
			vertex.position = vertices[i];
			vertex.normal = normals[i];
			vertex.texcoord = texcoords[i];
			vertex.tangent = tangents[i];
			//vertex.bitangent = bitangents[i];

			vertexData.push_back(vertex);  // 将每个顶点数据加入到数组中
		}

		return vertexData;
	}

	void setAttachNode(Node* node) {
		this->attachNode = node;
	}

	Node* getAttachNode() {
		return this->attachNode;
	}

protected:
	std::string name = "";

	Node* attachNode = nullptr;

	uint32_t vertexCount = 0;
	std::vector<Vec3> vertices;
	std::vector<Vec3> normals;
	std::vector<Vec3> tangents;
	std::vector<Vec3> bitangents;
	std::vector<Vec2> texcoords;

	uint32_t indexCount = 0;
	std::vector<uint32_t> indices;

	uint32_t faceCount = 0;

	AABB aabb;
};