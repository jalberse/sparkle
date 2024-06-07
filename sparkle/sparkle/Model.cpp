#include "Model.h"

namespace sparkle
{
	Model::Model(std::vector<std::shared_ptr<Mesh>>&& meshes, std::vector<std::shared_ptr<MeshRenderer>>&& meshRenderers)
		: m_meshes(std::move(meshes)), m_meshRenderers(std::move(meshRenderers))
	{
	}
}