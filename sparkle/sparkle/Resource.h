#pragma once

#include "stb_image.h"
#include "fmt/core.h"

#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Material.h"

namespace sparkle
{
	class Resource
	{
	public:
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

		static std::shared_ptr<Mesh> LoadMesh(const std::string& path)
		{
			if (meshCache.count(path) > 0)
			{
				return meshCache[path];
			}

			std::vector<glm::vec3> vertices;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> texCoords;
			std::vector<unsigned int> indices;

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
			aiMesh* mesh = scene->mMeshes[0];

			int drawCount = mesh->mNumVertices;

			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				// Note that this is a great instance of where Traits would be so great;
				// in Rust, we can simply implement From<aiVector3D> for glm::vec3.
				// Here, we don't own glm::vec3 so it's not as if we can implement a ctor for it from aiVector3D.
				// Helper functions or inlining it here is not as nice.

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
				if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
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
			auto result = std::make_shared<Mesh>(std::move(vertices), std::move(normals), std::move(texCoords), std::move(indices));
			meshCache[path] = result;
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

			std::string geometryCode;
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
				e;
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
			}
			return code;
		}

		static void ClearCache()
		{
			textureCache.clear();
			meshCache.clear();
			materialCache.clear();
		}

	private:
		static inline std::unordered_map<std::string, unsigned int> textureCache;
		static inline std::unordered_map<std::string, std::shared_ptr<Mesh>> meshCache;
		static inline std::unordered_map<std::string, std::shared_ptr<Material>> materialCache;

		static inline std::string defaultTexturePath = "Assets/Texture/";
		static inline std::string defaultMeshPath = "Assets/Model/";
		static inline std::string defaultMaterialPath = "Assets/Shader/";
	};
}