#include "Loader.hpp"
#include "Mesh.hpp"
#include "Node.hpp"
#include "Common/Math.hpp"

#include <iostream>
#include <string>

// ����ģ�Ͳ����ظ��ڵ�
Node* AssimpLoader::loadModel(const std::string& path) {
    // ʹ�� Assimp ����ģ��
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    // ����Ƿ���سɹ�
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    std::cout << "Loaded model: " << path << std::endl;

    // �ݹ鴦��ģ�͵����нڵ㲢���ظ��ڵ�
    return processNode(scene->mRootNode, scene);
}

// �ݹ鴦��ڵ�
Node* AssimpLoader::processNode(aiNode* node, const aiScene* scene) {
    // ����һ���µĽڵ�
    Node* sceneNode = new Node{};

    // ����ǰ�ڵ����������
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh* processedMesh = processMesh(mesh);
        if (processedMesh) {
            sceneNode->setName(processedMesh->getName());
            sceneNode->setMesh(processedMesh);
        }
    }

    // �ݹ鴦���ӽڵ�
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        Node* childNode = processNode(node->mChildren[i], scene);
        if (childNode) {
            sceneNode->addChild(childNode);
        }
    }

    return sceneNode;
}

// ���������񲢷��� Mesh ����
Mesh* AssimpLoader::processMesh(aiMesh* mesh) {
    std::string name = mesh->mName.C_Str();

    // ���� Mesh ����
    Mesh* sceneMesh = new Mesh{ name };

    // Ԥ�����ڴ�
    std::vector<Vec3> vertices;
    vertices.reserve(mesh->mNumVertices);

    std::vector<Vec3> normals;
    bool hasNormals = mesh->HasNormals();
    if (hasNormals) {
        normals.reserve(mesh->mNumVertices);
    }

    std::vector<Vec3> tangents;
    std::vector<Vec3> bitangents;
    bool hasTangents = mesh->HasTangentsAndBitangents();
    if (hasTangents) {
        tangents.reserve(mesh->mNumVertices);
        bitangents.reserve(mesh->mNumVertices);
    }

    std::vector<Vec2> texcoords;
    bool hasTexCoords = mesh->HasTextureCoords(0);
    if (hasTexCoords) {
        texcoords.reserve(mesh->mNumVertices);
    }

    std::vector<uint32_t> indices;
    indices.reserve(mesh->mNumFaces * 3); // ���趼��������

    // ��ȡ��������
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // ����λ��
        const aiVector3D& pos = mesh->mVertices[i];
        vertices.emplace_back(pos.x, pos.y, pos.z);

        // ����
        if (hasNormals) {
            const aiVector3D& normal = mesh->mNormals[i];
            normals.emplace_back(normal.x, normal.y, normal.z);
        }

        // ���ߺ͸�����
        if (hasTangents) {
            const aiVector3D& tangent = mesh->mTangents[i];
            tangents.emplace_back(tangent.x, tangent.y, tangent.z);
            const aiVector3D& bitangent = mesh->mBitangents[i];
            bitangents.emplace_back(bitangent.x, bitangent.y, bitangent.z);
        }

        // ��������
        if (hasTexCoords) {
            const aiVector3D& texCoord = mesh->mTextureCoords[0][i];
            texcoords.emplace_back(texCoord.x, texCoord.y);
        }
    }

    // ��ȡ��������
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        // Assimp ��֤��ÿ���涼��������
        indices.emplace_back(face.mIndices[0]);
        indices.emplace_back(face.mIndices[1]);
        indices.emplace_back(face.mIndices[2]);
    }

    // ���� Mesh ����
    sceneMesh->setVertices(std::move(vertices));
    if (hasNormals) {
        sceneMesh->setNormals(std::move(normals));
    }
    if (hasTangents) {
        sceneMesh->setTangents(std::move(tangents));
        sceneMesh->setBitangents(std::move(bitangents));
    }
    if (hasTexCoords) {
        sceneMesh->setTexcoords(std::move(texcoords));
    }
    sceneMesh->setIndices(std::move(indices));

    std::cout << "Processed Mesh: " << name
        << " | Vertices: " << vertices.size()
        << " | Indices: " << indices.size() << std::endl;

    return sceneMesh;
}