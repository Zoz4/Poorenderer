#pragma once

#include <vector>
#include <string>

#include <glm/vec3.hpp>

class aiScene;
class aiNode;
class aiMesh;

namespace Poorenderer {
	
	class Model
	{
	public:
		Model(const std::string& path);

	private:
		void LoadModel(const std::string& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		void ProcessMesh(aiMesh* mesh, const aiScene* scene);

	public:
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<uint32_t> indices;
	};
}