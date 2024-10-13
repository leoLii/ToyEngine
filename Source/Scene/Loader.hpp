#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

class Node;
class Mesh;

class AssimpLoader
{
public:
    Node* loadModel(const std::string& path);

private:
    Assimp::Importer importer;

    // 递归处理节点
    Node* processNode(aiNode* node, const aiScene* scene);

    // 处理网格并提取顶点数据
    Mesh* processMesh(aiMesh* mesh);
};