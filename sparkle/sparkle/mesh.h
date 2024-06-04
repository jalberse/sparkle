#pragma once

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <fmt/core.h>
#include <glm.hpp>

#include <vector>

namespace sparkle
{

class Mesh
{
public:
	/// <summary>
	/// Construct the Mesh from the given vertices and indices, first converting to GLM types.
	/// </summary>
	/// <param name="attributeSizes"></param>
	/// <param name="packedVertices"></param>
	/// <param name="indices"></param>
	Mesh(std::vector<unsigned int>&& attributeSizes,
		std::vector<float>&& packedVertices,
		std::vector<unsigned int>&& indices)
	{
		unsigned int stride = 0;
		for (unsigned int i = 0; i < attributeSizes.size(); i++)
		{
			stride += attributeSizes[i];
		}
		unsigned int numVertices = static_cast<unsigned int>(packedVertices.size()) / stride;

		for (unsigned int i = 0; i < numVertices; i++)
		{
			unsigned int baseV = stride * i;
			unsigned int baseN = (stride >= 6) ? baseV + 3 : baseV;
			unsigned int baseT = baseN + 3;

			m_positions.push_back(glm::vec3(packedVertices[baseV + 0], packedVertices[baseV + 1], packedVertices[baseV + 2]));
			if (stride >= 6)
			{
				m_normals.push_back(glm::vec3(packedVertices[baseN + 0], packedVertices[baseN + 1], packedVertices[baseN + 2]));
			}
			m_texCoords.push_back(glm::vec2(packedVertices[baseT + 0], packedVertices[baseT + 1]));
		}
		Initialize(m_positions, m_normals, m_texCoords, indices, attributeSizes);
	}

	Mesh(const std::vector<glm::vec3>&& vertices,
		const std::vector<glm::vec3>&& normals = std::vector<glm::vec3>(),
		const std::vector<glm::vec2>&& texCoords = std::vector<glm::vec2>(),
		const std::vector<unsigned int>&& indices = std::vector<unsigned int>())
	{
		Initialize(vertices, normals, texCoords, indices, std::vector<unsigned int>());
	}

	Mesh(const Mesh&) = delete;

	~Mesh()
	{
		if (m_EBO > 0)
		{
			glDeleteBuffers(1, &m_EBO);
		}
		if (!m_VBOs.empty())
		{
			glDeleteBuffers((GLsizei)m_VBOs.size(), m_VBOs.data());
		}
		if (m_VAO > 0)
		{
			glDeleteVertexArrays(1, &m_VAO);
		}
	}

	unsigned int VAO() const
	{
		if (m_VAO == 0)
		{
			fmt::print("Error(Mesh): Access VAO of 0 (possibly uninitialized");
		}
		return m_VAO;
	}

	bool useIndices() const
	{
		return m_indices.size() > 0;
	}

	unsigned int drawCount() const
	{
		if (useIndices())
		{
			return static_cast<unsigned int>(m_indices.size());
		}
		else {
			return static_cast<unsigned int>(m_positions.size());
		}
	}

	const std::vector<unsigned int>& indices() const
	{
		return m_indices;
	}

	const GLuint verticesVBO() const
	{
		return m_VBOs[0];
	}

	const GLuint normalsVBO() const
	{
		return m_VBOs[1];
	}

	void SetVerticesAndNormals(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals)
	{
		m_positions = vertices;
		m_normals = normals;
		glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[1]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);
	}

	/// <summary>
	/// Allocates a VBO and returns the handle.
	/// </summary>
	/// <param name="floatCount">The number of floats per vertex attribute; e.g. 3 for 3D points.</param>
	/// <param name="instanceAttribute"></param>
	/// <returns></returns>
	GLuint AllocateVBO(unsigned int floatCount, bool instanceAttribute = false)
	{
		GLuint VBO;
		glBindVertexArray(m_VAO);
		glGenBuffers(1, &VBO);
		m_VBOs.push_back(VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		int index = static_cast<int>(m_VBOs.size()) - 1;
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, floatCount, GL_FLOAT, GL_FALSE, floatCount * sizeof(float), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (instanceAttribute)
		{
			glVertexAttribDivisor(index, 1);
		}

		return VBO;
	}

private:
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_texCoords;
	std::vector<unsigned int> m_indices;

	GLuint m_VAO = 0;
	GLuint m_EBO = 0;
	// TODO Should we instead store vertex, normal, and texCoords VBOs as separate members?
	// Right now, this is populated by successive calls to AllocateVBO() - missing either normals or
	// texCoords could result in m_VBOs[1] referring to one or the other, which could be confusing.
	std::vector<GLuint> m_VBOs;

	void Initialize(
		const std::vector<glm::vec3>& vertices,
		const std::vector<glm::vec3>& normals,
		const std::vector<glm::vec2>& texCoords,
		const std::vector<unsigned int>& indices,
		const std::vector<unsigned int>& attributeSizes
	)
	{
		m_positions = vertices;
		m_normals = normals;
		m_texCoords = texCoords;
		m_indices = indices;

		// Bind Vertex Array Object
		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		// Copy our arrays into OpenGL buffers
		if (!m_positions.empty())
		{
			const GLuint vbo = AllocateVBO(3);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, m_positions.size() * sizeof(glm::vec3), m_positions.data(), GL_STATIC_DRAW);
		}
		if (!m_normals.empty())
		{
			const GLuint vbo = AllocateVBO(3);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), m_normals.data(), GL_STATIC_DRAW);
		}
		if (!m_texCoords.empty())
		{
			const GLuint vbo = AllocateVBO(2);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, m_texCoords.size() * sizeof(glm::vec2), m_texCoords.data(), GL_STATIC_DRAW);
		}
		// Unbind to be safe
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Copy our index array in an element buffer
		if (useIndices())
		{
			glGenBuffers(1, &m_EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
		}
		// Unbind to be safe
		glBindVertexArray(0);
	}
};

}
