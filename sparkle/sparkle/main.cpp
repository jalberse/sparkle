
#include <iostream>

#include "SpEngine.h"
#include "GameInstance.h"
#include "Resource.h"
#include "Scene.h"
#include "utils.h"

namespace sparkle
{
	class ScenePrimitiveRendering : public Scene
	{
	public:
		ScenePrimitiveRendering()
		{
			name = "Primitive Rendering";
		}

		void PopulateActors(GameInstance* game)
		{
			Scene::SpawnCameraAndLight(game);

			auto material = Resource::LoadMaterial("_Default");
			{
				material->Use();
				material->SetTexture("material.diffuse", Resource::LoadTexture("wood.png"));
				material->SetBool("material.useTexture", true);
			}

			std::shared_ptr<Actor> sphere = game->CreateActor("Sphere");
			{
				auto mesh = Resource::LoadMesh("sphere.obj");
				auto renderer = std::make_shared<MeshRenderer>(mesh, material, true);
				sphere->AddComponent(renderer);
				sphere->transform->position = glm::vec3(0.6f, 2.0f, 0.0f);
				sphere->transform->scale = glm::vec3(0.5f);
			}
		}
	};
}

int main()
{
	// TODO Well, it was working and then I removed vcpkg (I thought I didn't need it anymore?)
	//  but now we can't open assimp files (is it storing them in the vcpkg directory?).
	//  So, anyways, re-install assimp and then I should have a pretty neat render engine to work with.
	// vcpkg recommends using it as a submodule, so do that (what I had before, dummy...)
	// add it, and add it as a submodule.

	auto engine = std::make_unique<sparkle::SpEngine>();

	std::vector<std::shared_ptr<sparkle::Scene>> scenes = {
		std::make_shared<sparkle::ScenePrimitiveRendering>(),
	};

	engine->SetScenes(scenes);

	return engine->Run();
}