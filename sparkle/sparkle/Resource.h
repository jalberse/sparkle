#pragma once

#include "stb_image.h"
#include "fmt/core.h"

#include <utility>
#include <optional>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.h"
#include "Mesh.h"
#include "Material.h"

namespace sparkle
{
	class Resource
	{
	public:
		// TODO Accept filesystem path instead.
		static unsigned int LoadTexture(const std::string& path)
		{
			if(textureCache.count(path) > 0)
			{
				return textureCache[path];
			}

			unsigned int textureID;
			glGenTextures(1, &textureID);
			textureCache[path] = textureID;

			int width, height, nrComponents;
			unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
			if (data == nullptr)
			{
				data = stbi_load((defaultTexturePath + path).c_str(), &width, &height, &nrComponents, 0);
			}
			if (data)
			{
				GLenum internalFormat = GL_RED;
				GLenum dataFormat = GL_RED;

				if (nrComponents == 3)
				{
					internalFormat = GL_SRGB;
					dataFormat = GL_RGB;
				}
				else if (nrComponents == 4)
				{
					internalFormat = GL_SRGB_ALPHA;
					dataFormat = GL_RGBA;
				}

				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				stbi_image_free(data);
			}
			else
			{
				fmt::print("Error(Resource): Texture failed to load at path({})\n", path);
				stbi_image_free(data);
			}
			return textureID;
		}

		/// <summary>
		/// Loads the mesh at the specified path.
		/// Assumes that the mesh is the first mesh in the scene, and does not load other meshes in the scene.
		/// Does not load any materials.
		/// To load all meshes at the path as a single model, use LoadModel().
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		static std::shared_ptr<Mesh> LoadMesh(const std::string& path)
		{
			if (meshCache.count(path) > 0)
			{
				return meshCache[path];
			}

			
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(defaultMeshPath + path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
			// check for errors
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

				if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
				{
					fmt::print("Error(Resource): Assimp error({})\n", importer.GetErrorString());
					exit(-1);
				}
			}

			aiMesh* aiMesh = scene->mMeshes[0];

			std::shared_ptr<Mesh> mesh = ProcessMesh(aiMesh, scene);

			meshCache[path] = mesh;
			return mesh;
		}
		
		/// <summary>
		/// Loads a model at the specified path, including its meshes and textures.
		/// A MeshRenderer is created for each Mesh in the model using the input material,
		/// Each MeshRenderer's MaterialProperty additionally configured for each mesh where applicable.
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		static std::shared_ptr<Model> LoadModel(const std::string& path, const std::shared_ptr<Material>& material, bool flipUVs = true)
		{
			if (modelCache.count(path) > 0)
			{
				return modelCache[path];
			}

			std::filesystem::path p(path);
			if (!std::filesystem::exists(p))
			{
				fmt::print("Error(Resource): Model not found at path({})\n", path);
				exit(-1);
			}
			std::filesystem::path modelDir = p.parent_path();

			std::vector<std::shared_ptr<Mesh>> modelMeshes{};
			std::vector<std::shared_ptr<MeshRenderer>> meshRenderers{};

			unsigned int flipUV = flipUVs ? aiProcess_FlipUVs : 0;

			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | flipUV | aiProcess_CalcTangentSpace);
			// check for errors
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				fmt::println("ERROR::ASSIMP::{}", importer.GetErrorString());
				exit(-1);
			}

			// process ASSIMP's root node recursively, adding to the modelMeshes and modelMaterials vectors
			ProcessNode(scene->mRootNode, scene, material, modelMeshes, meshRenderers, modelDir);

			auto result = std::make_shared<Model>(std::move(modelMeshes), std::move(meshRenderers));
			modelCache[path] = result;
			return result;
		}

		static std::shared_ptr<Material> LoadMaterial(const std::string& path, bool includeGeometryShader = false)
		{
			if (materialCache.count(path) > 0)
			{
				return materialCache[path];
			}
			std::string vertexCode = LoadText(defaultMaterialPath + path + ".vert");
			if (vertexCode.empty())
			{
				vertexCode = LoadText(path + ".vert");
			}
			if (vertexCode.empty())
			{
				fmt::print("Error(Resource): Vertex shader not found {}\n", path);
				exit(-1);
			}

			std::string fragmentCode = LoadText(defaultMaterialPath + path + ".frag");
			if (fragmentCode.empty())
			{
				fragmentCode = LoadText(path + ".frag");
			}
			if (fragmentCode.empty())
			{
				fmt::print("Error(Resource): Fragment shader not found {}\n", path);
				exit(-1);
			}

			std::string geometryCode = "";
			if (includeGeometryShader)
			{
				geometryCode = LoadText(defaultMaterialPath + path + ".geom");
				if (geometryCode.empty())
				{
					geometryCode = LoadText(path + ".geom");
				}
				if (geometryCode.empty())
				{
					fmt::print("Error(Resource): Geometry shader not found {}\n", path);
					exit(-1);
				}
			}

			std::shared_ptr<Material> result = std::make_shared<Material>(vertexCode, fragmentCode, geometryCode);
			materialCache[path] = result;
			result->name = path;
			return result;
		}

		/// <summary>
		/// </summary>
		/// <param name="path"></param>
		/// <returns>An empty string if loading failed; else the text of the file</returns>
		static std::string LoadText(const std::string& path)
		{
			// 1. retrieve the vertex/fragment source code from filePath
			std::string code;
			std::ifstream file;
			// ensure ifstream objects can throw exceptions:
			file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{
				// open files
				file.open(path);
				std::stringstream vShaderStream;
				// read file's buffer contents into streams
				vShaderStream << file.rdbuf();
				// close file handlers
				file.close();
				// convert stream into string
				code = vShaderStream.str();
			}
			catch (std::ifstream::failure& e)
			{
				return "";
			}
			return code;
		}

		static void ClearCache()
		{
			for(auto& [key, value] : textureCache)
			{
				glDeleteTextures(1, &value);
			}

			textureCache.clear();
			meshCache.clear();
			modelCache.clear();
			materialCache.clear();
		}

	private:
		static void ProcessNode(
			aiNode* node,
			const aiScene* scene,
			const std::shared_ptr<Material>& mat,
			std::vector<std::shared_ptr<Mesh>>& modelMeshes,
			std::vector<std::shared_ptr<MeshRenderer>>& modelMeshRenderers,
			const std::filesystem::path& modelDir)
		{
			// process all the node's meshes (if any)
			for (unsigned int i = 0; i < node->mNumMeshes; i++)
			{
				aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];
				std::shared_ptr<Mesh> mesh = ProcessMesh(aiMesh, scene);
				modelMeshes.push_back(mesh);

				std::optional<MaterialProperty> materialProperty = ProcessMeshMaterial(aiMesh, scene, modelDir);
				MeshRenderer meshRenderer(modelMeshes.back(), mat, true);
				if (materialProperty.has_value())
				{
					meshRenderer.SetMaterialProperty(*materialProperty);
				}
				modelMeshRenderers.push_back(std::make_shared<MeshRenderer>(meshRenderer));
			}
			// then do the same for each of its children
			for (unsigned int i = 0; i < node->mNumChildren; i++)
			{
				ProcessNode(node->mChildren[i], scene, mat, modelMeshes, modelMeshRenderers, modelDir);
			}
		}

		static std::shared_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene)
		{
			std::vector<glm::vec3> vertices{};
			std::vector<glm::vec3> normals{};
			std::vector<glm::vec2> texCoords{};
			std::vector<unsigned int> indices{};
			int drawCount = mesh->mNumVertices;

			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				// positions
				vertices.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));

				// normals
				if (mesh->HasNormals())
				{
					normals.push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
				}
				else {
					fmt::print("Normals not found\n");
					exit(-1);
				}
				// Texture coordinates
				if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
				{
					glm::vec2 texCoord;
					texCoord.x = mesh->mTextureCoords[0][i].x;
					texCoord.y = mesh->mTextureCoords[0][i].y;
					texCoords.push_back(texCoord);
				}
				else {
					texCoords.push_back(glm::vec2(0.0f, 0.0f));
				}
			}

			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
				{
					indices.push_back(face.mIndices[j]);
				}
			}

			return std::make_shared<Mesh>(std::move(vertices), std::move(normals), std::move(texCoords), std::move(indices));
		}

		static std::optional<MaterialProperty> ProcessMeshMaterial(aiMesh* mesh, const aiScene* scene, const std::filesystem::path& modelDir)
		{
			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				std::vector<unsigned int> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, modelDir);
				std::vector<unsigned int> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, modelDir);

				if (diffuseMaps.size() == 0 && specularMaps.size() == 0)
				{
					return {};
				}

				MaterialProperty materialProperty{};
				materialProperty.preRendering = [diffuseMaps, specularMaps](Material* mat)
				{
					if (diffuseMaps.size() > 0)
					{
						mat->SetBool("material.useTexture", true);
						mat->SetTexture("material.diffuse", diffuseMaps[0]);
					}
					if (specularMaps.size() > 0)
					{
						// TODO Our shader doesn't use a specular map yet! Add that.
						// mat->SetBool("material.useTexture", true);
						// mat->SetTexture("material.specular", specularMaps[0]);
					}
				};

				if (diffuseMaps.size() > 1 || specularMaps.size() > 1)
				{
					fmt::print("ERROR(Resource): Multiple textures not supported, but multiple found in model loading.\n");
				}

				return materialProperty;
			}
			else return {};
		}

		static std::vector<unsigned int> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::filesystem::path& modelDir)
		{
			std::vector<unsigned int> textures;
			for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
			{
				aiString str;
				mat->GetTexture(type, i, &str);
				const std::filesystem::path p = modelDir / str.C_Str();
				unsigned int id = Resource::LoadTexture(p.string());
				textures.push_back(id);
			}
			return textures;
		}

		static inline std::unordered_map<std::string, unsigned int> textureCache;
		static inline std::unordered_map<std::string, std::shared_ptr<Mesh>> meshCache;
		static inline std::unordered_map<std::string, std::shared_ptr<Material>> materialCache;
		static inline std::unordered_map<std::string, std::shared_ptr<Model>> modelCache;

		static inline std::string defaultTexturePath = "Assets/Texture/";
		static inline std::string defaultMeshPath = "Assets/Model/";
		static inline std::string defaultMaterialPath = "Assets/Shader/";
	};
}