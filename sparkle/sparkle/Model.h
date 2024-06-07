#pragma once

#include <vector>
#include <memory>

#include "Mesh.h"
#include "MeshRenderer.h"

namespace sparkle
{

// Bundles constituent meshes and their renderers when loading a model from disk.
// Not a component itself; the contained mesh renderers should be added to the actor instead,
// so they can be discovered for rendering (we don't like nested components!)
struct Model
{
	Model(std::vector<std::shared_ptr<Mesh>>&& meshes, std::vector<std::shared_ptr<MeshRenderer>>&& meshRenderers);

	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::vector<std::shared_ptr<MeshRenderer>> m_meshRenderers;
};

}