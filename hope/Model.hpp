#pragma once

#include <vector>
#include <string>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Mesh.hpp"
#include "Texture.hpp"

class Model {
public:
	Model() {};
	~Model() { clearModel(); };
	void loadModel(const std::string& path);
	void renderModel();
	void clearModel();
private:
	std::vector<Mesh*> meshList{};
	std::vector<Texture*> textureList{};
	std::vector<unsigned int> meshToTexture{};

	void loadNode(aiNode* node, const aiScene* scene);
	void loadMesh(aiMesh* mesh, const aiScene* scene);
	void loadMaterials(const aiScene* scene);
};