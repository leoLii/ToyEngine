#include "Loader.hpp"
#include "Mesh.hpp"
#include "Node.hpp"
#include "Common/Math.hpp"

#include <iostream>
#include <string>

// 加载模型并返回根节点
Node* AssimpLoader::loadModel(const std::string& path) {
    // 使用 Assimp 导入模型
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    // 检查是否加载成功
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    std::cout << "Loaded model: " << path << std::endl;

    // 递归处理模型的所有节点并返回根节点
    return processNode(scene->mRootNode, scene);
}

// 递归处理节点
Node* AssimpLoader::processNode(aiNode* node, const aiScene* scene) {
    // 创建一个新的节点
    Node* sceneNode = new Node{};

    // 处理当前节点的所有网格
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh* processedMesh = processMesh(mesh);
        if (processedMesh) {
            sceneNode->setName(processedMesh->getName());
            sceneNode->setMesh(processedMesh);
        }
    }

    // 递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        Node* childNode = processNode(node->mChildren[i], scene);
        if (childNode) {
            sceneNode->addChild(childNode);
        }
    }

    return sceneNode;
}

// 处理单个网格并返回 Mesh 对象
Mesh* AssimpLoader::processMesh(aiMesh* mesh) {
    std::string name = mesh->mName.C_Str();

    // 创建 Mesh 对象
    Mesh* sceneMesh = new Mesh{ name };

    // 预分配内存
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
    indices.reserve(mesh->mNumFaces * 3); // 假设都是三角形

    // 提取顶点数据
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // 顶点位置
        const aiVector3D& pos = mesh->mVertices[i];
        vertices.emplace_back(pos.x, pos.y, pos.z);

        // 法线
        if (hasNormals) {
            const aiVector3D& normal = mesh->mNormals[i];
            normals.emplace_back(normal.x, normal.y, normal.z);
        }

        // 切线和副切线
        if (hasTangents) {
            const aiVector3D& tangent = mesh->mTangents[i];
            tangents.emplace_back(tangent.x, tangent.y, tangent.z);
            const aiVector3D& bitangent = mesh->mBitangents[i];
            bitangents.emplace_back(bitangent.x, bitangent.y, bitangent.z);
        }

        // 纹理坐标
        if (hasTexCoords) {
            const aiVector3D& texCoord = mesh->mTextureCoords[0][i];
            texcoords.emplace_back(texCoord.x, texCoord.y);
        }
    }

    // 提取索引数据
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        // Assimp 保证了每个面都是三角形
        indices.emplace_back(face.mIndices[0]);
        indices.emplace_back(face.mIndices[1]);
        indices.emplace_back(face.mIndices[2]);
    }

    // 设置 Mesh 数据
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