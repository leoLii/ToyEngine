#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>


class AssimpLoader
{
public:

    /*  ����   */
    bool loadModel(const std::string& path);

private:
    Assimp::Importer importer;

    // �ݹ鴦��ڵ�
    void processNode(aiNode* node, const aiScene* scene);

    // ����������ȡ��������
    void processMesh(aiMesh* mesh);
};