#include "Loader.hpp"

#include <iostream>

bool AssimpLoader::loadModel(const std::string& path) {
    // 使用 Assimp 导入模型
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    // 检查是否加载成功
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return false;
    }

    std::cout << "Loaded model: " << path << std::endl;

    // 递归处理模型的所有节点
    processNode(scene->mRootNode, scene);
    return true;
}

void AssimpLoader::processNode(aiNode* node, const aiScene* scene) {
    // 处理当前节点的所有网格
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh);
    }

    // 递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

// 处理网格并提取顶点数据
void AssimpLoader::processMesh(aiMesh* mesh) {
    std::cout << "Processing mesh with " << mesh->mNumVertices << " vertices." << std::endl;

    // 提取顶点数据
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // 获取顶点位置
        aiVector3D pos = mesh->mVertices[i];
        std::cout << "Vertex " << i << ": (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;

        // 如果有法线数据
        if (mesh->HasNormals()) {
            aiVector3D normal = mesh->mNormals[i];
            std::cout << "  Normal: (" << normal.x << ", " << normal.y << ", " << normal.z << ")" << std::endl;
        }

        // 如果有纹理坐标数据
        if (mesh->mTextureCoords[0]) {
            aiVector3D texCoord = mesh->mTextureCoords[0][i];
            std::cout << "  TexCoord: (" << texCoord.x << ", " << texCoord.y << ")" << std::endl;
        }
    }

    // 提取索引数据
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        std::cout << "Face " << i << ": ";
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            std::cout << face.mIndices[j] << " ";
        }
        std::cout << std::endl;
    }
}