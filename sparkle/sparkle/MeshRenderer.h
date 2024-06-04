#pragma once

#include "Component.h"
#include "Mesh.h"
#include "Material.h"
#include "MaterialProperty.h"

#include <memory>

namespace sparkle
{
	class MeshRenderer : public Component
	{
	public:
		MeshRenderer(
			std::shared_ptr<Mesh> mesh,
			std::shared_ptr<Material> material,
			bool castShadow = false
		);

		void SetMaterialProperty(const MaterialProperty& materialProperty);

		virtual void Render(glm::mat4 lightMatrix);

		virtual void RenderShadow(glm::mat4 lightMatrix);

		virtual void DrawCall();

		std::shared_ptr<Material> material() const;

		std::shared_ptr<Mesh> mesh() const
		{
			return m_mesh;
		}
		
	protected:
		void SetupLighting(std::shared_ptr<Material> material);

		int m_numInstances = 0;
		std::shared_ptr<Mesh> m_mesh;
		std::shared_ptr<Material> m_material;
		std::shared_ptr<Material> m_shadowMaterial;
		MaterialProperty m_materialProperty;
	};
}