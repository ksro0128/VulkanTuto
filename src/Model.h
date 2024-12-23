#pragma once

#include "Common.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
public:
	static std::shared_ptr<Model> createModel(std::string path);
	static std::shared_ptr<Model> createBoxModel();
	static std::shared_ptr<Model> createSphereModel();
	static std::shared_ptr<Model> createPlaneModel();
	~Model() {}
	void cleanup();

	void draw(VkCommandBuffer commandBuffer);
private:
	Model() {}

	std::vector< std::shared_ptr<Mesh> > m_meshes;

	void initModel(std::string path);
	void initBoxModel();
	void initSphereModel();
	void initPlaneModel();
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);
};
