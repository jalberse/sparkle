
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

	class SceneBackpack : public Scene
	{
	public:
		SceneBackpack()
		{
			name = "Backpack";
		}

		void PopulateActors(GameInstance* game)
		{
			Scene::SpawnCameraAndLight(game);

			// This is a model with its material defined, so we use that to set the MaterialProperties for this
			// material when as we load it. No need to specify the defaults here.
			std::shared_ptr<Material> material = Resource::LoadMaterial("_Default");

			std::shared_ptr<Actor> backpack = game->CreateActor("Backpack");
			{
				std::shared_ptr<Model> model = Resource::LoadModel("Assets/backpack/backpack.obj", material);
				for(auto meshRenderer : model->m_meshRenderers)
				{
					backpack->AddComponent(meshRenderer);
				}
				backpack->transform->position = glm::vec3(0.0f, 0.0f, 0.0f);
				backpack->transform->scale = glm::vec3(1.0f);
			}
		}
	};
}

int main()
{
	auto engine = std::make_unique<sparkle::SpEngine>();

	std::vector<std::shared_ptr<sparkle::Scene>> scenes = {
		std::make_shared<sparkle::ScenePrimitiveRendering>(),
		std::make_shared<sparkle::SceneBackpack>(),
	};

	engine->SetScenes(scenes);

	return engine->Run();
}