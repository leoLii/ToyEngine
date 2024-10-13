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

    // �ݹ鴦��ڵ�
    Node* processNode(aiNode* node, const aiScene* scene);

    // ����������ȡ��������
    Mesh* processMesh(aiMesh* mesh);
};