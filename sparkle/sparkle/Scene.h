#pragma once

#include <string>
#include <functional>
#include <memory>

#include "GameInstance.h"
#include "Input.h"
#include "Resource.h"
#include "Actor.h"
#include "PlayerController.h"
#include "MeshRenderer.h"
#include "MaterialProperty.h"
#include "Light.h"
#include "Camera.h"
#include "Component.h"

namespace sparkle
{
	class Scene
	{
	public:
		std::string name = "BaseScene";

		virtual void PopulateActors(GameInstance* game) = 0;

		void ClearCallbacks() {
			onEnter.Clear();
			onExit.Clear();
		}

		SpCallback<void()> onEnter;
		SpCallback<void()> onExit;

	protected:
		template <class T>
		void ModifyParameter(T* ptr, T value)
		{
			onEnter.Register([this, ptr, value]() {
				T prev = *ptr;
				*ptr = value;
				onExit.Register([ptr, prev, value]() {
					*ptr = prev;
					});
				});
		}

		// Spawn*() functions are utility functions to spawn common actors.
		// They create the actor, which is owned by the GameInstance, and add necessary components,
		// returning a shared_ptr to the new actor.

		std::shared_ptr<Actor> SpawnCamera(GameInstance* game)
		{
			auto actor = game->CreateActor("Prefab Camera");
			auto camera = std::make_shared<Camera>();
			auto controller = std::make_shared<PlayerController>();
			actor->AddComponents({ camera, controller });
			return actor;
		}

		std::shared_ptr<Actor> SpawnLight(GameInstance* game)
		{
			auto actor = game->CreateActor("Prefab Light");
			auto mesh = Resource::LoadMesh("cylinder.obj");
			auto material = Resource::LoadMaterial("Assets/Shader/UnlitWhite");
			auto renderer = std::make_shared<MeshRenderer>(mesh, material);
			auto light = std::make_shared<Light>();

			actor->AddComponents({ renderer, light });
			return actor;
		}

		void SpawnCameraAndLight(GameInstance* game)
		{
			auto camera = SpawnCamera(game);
			camera->Initialize(glm::vec3(0.35, 3.3, 7.2),
				glm::vec3(1),
				glm::vec3(-21, 2.25, 0));

			auto light = SpawnLight(game);
			light->Initialize(glm::vec3(2.5f, 5.0f, 2.5f),
				glm::vec3(0.25f),
				glm::vec3(20.0f, 30.0f, 0.0f));
		}
	};
}