#pragma once

#include <vector>
#include <string>
#include "Common/Math.hpp"

class Mesh {
public:
	Mesh() = default;
	Mesh(std::string& name);

	~Mesh() = default;

	void setVertices(std::vector<Vec3>&&);
	void setNormals(std::vector<Vec3>&&);
	void setTangents(std::vector<Vec3>&&);
	void setBitangents(std::vector<Vec3>&&);
	void setTexcoords(std::vector<Vec2>&&);
	void setIndices(std::vector<uint32_t>&&);

protected:
	std::string name = "";

	uint32_t vertexCount = 0;
	std::vector<Vec3> vertices;
	std::vector<Vec3> normals;
	std::vector<Vec3> tangents;
	std::vector<Vec3> bitangents;
	std::vector<Vec2> texcoords;

	uint32_t indexCount = 0;
	std::vector<uint32_t> indices;

	uint32_t faceCount = 0;
};