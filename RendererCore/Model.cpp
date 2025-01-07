#include "Model.h"
#include "logger.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>


namespace Poorenderer {
	Model::Model(const std::string& path)
	{
		LoadModel(path);
		LOGD("TotalNumVertices: {}", vertices.size());
		LOGD("TotalNumNormals: {}", normals.size());
		LOGD("TotalNumIndices: {}", indices.size());

		if (indices.size() % 3 != 0) {
			LOGE("Only Support Triangles");
		}
	}
	void Model::LoadModel(const std::string& path)
	{
		Assimp::Importer importer;
		// aiProcess_GenSmoothNormals | aiProcess_GenNormals
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{	
			LOGE("ASSIMP:: {}", importer.GetErrorString());
			return;
		}
		LOGD("SceneNumMeshes: {}", scene->mNumMeshes);
		ProcessNode(scene->mRootNode, scene);
		LOGI("Load Model: {} Success!", scene->mRootNode->mName.C_Str());
	}
    void Model::ProcessNode(aiNode* node, const aiScene* scene)
    {
        LOGD("\tNodeName: {}", node->mName.C_Str());
        LOGD("\tNumMesh: {}", node->mNumMeshes);
        LOGD("\tNumChildren: {}", node->mNumChildren);
        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(mesh, scene);
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }
	void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		LOGD("\t\tMeshName: {}", mesh->mName.C_Str());
		LOGD("\t\tNumVertices: {}", mesh->mNumVertices);
		LOGD("\t\tNumFaces: {}", mesh->mNumFaces);

		// 记录前一个Mesh中的顶点数量，用于偏移顶点索引，
		// 实现将不同Mesh的所有顶点整合到一个顶点数组的同时维护索引关系的正确性
		unsigned int numVertices = vertices.size();
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			vertices.emplace_back(
				mesh->mVertices[i].x,
				mesh->mVertices[i].y,
				mesh->mVertices[i].z
			);

			if ( mesh->HasNormals() )
			{
				normals.emplace_back(
					mesh->mNormals[i].x,
					mesh->mNormals[i].y,
					mesh->mNormals[i].z
				);
			}
		}
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(numVertices+face.mIndices[j]);
		}
	}
}