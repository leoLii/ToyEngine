#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>


class AssimpLoader
{
public:

    /*  函数   */
    bool loadModel(const std::string& path);

private:
    Assimp::Importer importer;

    // 递归处理节点
    void processNode(aiNode* node, const aiScene* scene);

    // 处理网格并提取顶点数据
    void processMesh(aiMesh* mesh);
};