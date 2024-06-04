
#include "MeshRenderer.h"
#include "Resource.h"
#include "Global.h"
#include "Light.h"
#include "GameInstance.h"
#include "Camera.h"

namespace sparkle
{
	MeshRenderer::MeshRenderer(
		std::shared_ptr<Mesh> mesh,
		std::shared_ptr<Material> material,
		bool castShadow
	) : m_mesh(mesh), m_material(material)
	{
		SET_COMPONENT_NAME;

		if (castShadow)
		{
			m_shadowMaterial = Resource::LoadMaterial("_ShadowDepth");
		}
	}

	void MeshRenderer::SetMaterialProperty(const MaterialProperty& materialProperty)
	{
		m_materialProperty = materialProperty;
	}

	// TODO Support light types other than spotlights.
	void MeshRenderer::SetupLighting(std::shared_ptr<Material> material)
	{
		if (Global::lights.size() == 0)
		{
			return;
		}
		const auto light = Global::lights[0];
		const std::string& prefix = fmt::format("spotLight.");
		const auto front = utils::RotateEuler(glm::vec3(0, -1, 0), light->transform()->rotation);

		m_material->SetVec3(prefix + "position", light->position());
		m_material->SetVec3(prefix + "direction", front);
		m_material->SetFloat(prefix + "cutOff", glm::cos(glm::radians(light->innerCutoff)));
		m_material->SetFloat(prefix + "outerCutOff", glm::cos(glm::radians(light->outerCutoff)));

		m_material->SetFloat(prefix + "constant", light->constant);
		m_material->SetFloat(prefix + "linear", light->linear);
		m_material->SetFloat(prefix + "quadratic", light->quadratic);

		m_material->SetVec3(prefix + "color", light->color);
		m_material->SetFloat(prefix + "ambient", light->ambient);
	}

	void MeshRenderer::Render(glm::mat4 lightMatrix)
	{
		if (m_material->noWireframe && Global::gameState.renderWireframe)
		{
			return;
		}

		m_material->Use();

		// Material
		m_material->SetFloat("material.specular", m_material->specular);
		m_material->SetFloat("material.smoothness", m_material->smoothness);
		m_material->SetTexture("_ShadowTex", Global::game->depthTex());

		if (m_materialProperty.preRendering)
		{
			m_materialProperty.preRendering(m_material.get());
		}

		// Camera param
		m_material->SetVec3("_CameraPos", Global::camera->transform()->position);

		// Light params
		SetupLighting(m_material);

		// texture
		int i = 0;
		for (auto tex : m_material->textures)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, tex.second);
			m_material->SetInt(tex.first, i);
			i++;
		}

		// matrices
		auto model = actor->transform->matrix();
		auto view = Global::camera->view();
		auto projection = Global::camera->projection();

		m_material->SetMat4("_Model", model);
		m_material->SetMat4("_View", view);
		m_material->SetMat4("_Projection", projection);

		// Not all shaders will use all of these uniforms.
		// If this shader does not have this uniform, it will be ignored.
		// Note that most shaders will need to do lighting, and thus need the individual m/v/p matrices.
		// But some shaders don't, so they can accept a combined matrix for a performance boost.
		m_material->SetMat4("_MVP", projection * view * model);
		m_material->SetMat4("_InvView", glm::inverse(view));
		m_material->SetMat3("_NormalMatrix", glm::transpose(glm::inverse(model)));

		m_material->SetMat4("_WorldToLight", lightMatrix);

		DrawCall();
	}

	void MeshRenderer::RenderShadow(glm::mat4 lightMatrix)
	{
		if (m_shadowMaterial == nullptr)
		{
			return;
		}

		m_shadowMaterial->Use();
		m_shadowMaterial->SetMat4("_Model", actor->transform->matrix());
		m_shadowMaterial->SetMat4("_WorldToLight", lightMatrix);

		DrawCall();
	}

	void MeshRenderer::DrawCall()
	{
		if (m_material->doubleSided)
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_mesh->VAO());
		if (m_mesh->useIndices())
		{
			if (m_numInstances > 0)
			{
				glDrawElementsInstanced(GL_TRIANGLES, m_mesh->drawCount(), GL_UNSIGNED_INT, 0, m_numInstances);
			}
			else {
				glDrawElements(GL_TRIANGLES, m_mesh->drawCount(), GL_UNSIGNED_INT, 0);
			}
		}
		else
		{
			if (m_numInstances > 0)
			{
				glDrawArraysInstanced(GL_TRIANGLES, 0, m_mesh->drawCount(), m_numInstances);
			}
			else
			{
				glDrawArrays(GL_TRIANGLES, 0, m_mesh->drawCount());
			}
		}

		// Reset face culling
		if (m_material->doubleSided)
		{
			glEnable(GL_CULL_FACE);
		}
	}

	std::shared_ptr<Material> MeshRenderer::material() const
	{
		return m_material;
	}
}