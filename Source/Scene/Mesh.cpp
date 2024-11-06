#include "Mesh.hpp"

Mesh::Mesh(std::string& name)
	:name{ name }
{
}

void Mesh::setVertices(std::vector<Vec3>&& vertices)
{
	this->vertices = vertices;
}

void Mesh::setNormals(std::vector<Vec3>&& normals)
{
	this->normals = normals;
}

void Mesh::setTangents(std::vector<Vec3>&& tangents)
{
	this->tangents = tangents;
}

void Mesh::setBitangents(std::vector<Vec3>&& bitangents)
{
	this->bitangents = bitangents;
}

void Mesh::setTexcoords(std::vector<Vec2>&& texcoords)
{
	this->texcoords = texcoords;
}

void Mesh::setIndices(std::vector<uint32_t>&& indices)
{
	this->indices = indices;
}

void Mesh::updateAABB()
{
	for (auto vertex : vertices) {
		aabb.min = Vec3(glm::min(vertex, aabb.min));
		aabb.max = Vec3(glm::max(vertex, aabb.max));
	}
}
