#include "Loader.hpp"

#include <iostream>

bool AssimpLoader::loadModel(const std::string& path) {
    // ʹ�� Assimp ����ģ��
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    // ����Ƿ���سɹ�
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return false;
    }

    std::cout << "Loaded model: " << path << std::endl;

    // �ݹ鴦��ģ�͵����нڵ�
    processNode(scene->mRootNode, scene);
    return true;
}

void AssimpLoader::processNode(aiNode* node, const aiScene* scene) {
    // ����ǰ�ڵ����������
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh);
    }

    // �ݹ鴦���ӽڵ�
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

// ����������ȡ��������
void AssimpLoader::processMesh(aiMesh* mesh) {
    std::cout << "Processing mesh with " << mesh->mNumVertices << " vertices." << std::endl;

    // ��ȡ��������
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // ��ȡ����λ��
        aiVector3D pos = mesh->mVertices[i];
        std::cout << "Vertex " << i << ": (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;

        // ����з�������
        if (mesh->HasNormals()) {
            aiVector3D normal = mesh->mNormals[i];
            std::cout << "  Normal: (" << normal.x << ", " << normal.y << ", " << normal.z << ")" << std::endl;
        }

        // �����������������
        if (mesh->mTextureCoords[0]) {
            aiVector3D texCoord = mesh->mTextureCoords[0][i];
            std::cout << "  TexCoord: (" << texCoord.x << ", " << texCoord.y << ")" << std::endl;
        }
    }

    // ��ȡ��������
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        std::cout << "Face " << i << ": ";
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            std::cout << face.mIndices[j] << " ";
        }
        std::cout << std::endl;
    }
}