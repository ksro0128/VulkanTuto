#include "Model.h"

std::shared_ptr<Model> Model::createModel(std::string path) {
    std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
    model->initModel(path);
    return model;
}


std::shared_ptr<Model> Model::createBoxModel() {
    std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
    model->initBoxModel();
    return model;
}


std::shared_ptr<Model> Model::createSphereModel() {
    std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
    model->initSphereModel();
    return model;
}


std::shared_ptr<Model> Model::createPlaneModel() {
    std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
    model->initPlaneModel();
    return model;
}


void Model::cleanup() {
    for (auto& mesh : m_meshes) {
        mesh->cleanup();
    }
}


void Model::draw(VkCommandBuffer commandBuffer) {
    for (auto& mesh : m_meshes) {
        mesh->draw(commandBuffer);
    }
}


void Model::initModel(std::string path) {
    loadModel(path);
}


void Model::initBoxModel() {
    m_meshes.push_back(Mesh::createBox());
}


void Model::initSphereModel() {
    m_meshes.push_back(Mesh::createSphere());
}


void Model::initPlaneModel() {
    m_meshes.push_back(Mesh::createPlane());
}


void Model::loadModel(std::string path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("failed to load model!");
    }

    processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(std::move(processMesh(mesh, scene)));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}


std::shared_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};
        vertex.pos = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        vertex.normal = {1.0f, 1.0f, 1.0f};
        if (mesh->mTextureCoords[0]) {
            vertex.texCoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        } else {
            vertex.texCoord = {0.0f, 0.0f};
        }
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return Mesh::createMesh(vertices, indices);
}
