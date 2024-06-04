#pragma once

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fmt/core.h>

#include "Component.h"
#include "Transform.h"

namespace sparkle
{
	class Actor
	{
	public:
		Actor();

		Actor(const std::string name);

		void Initialize(glm::vec3 position,
			glm::vec3 scale = glm::vec3(1),
			glm::vec3 rotation = glm::vec3(0));

		void Start();

		void Update();

		void FixedUpdate();

		void OnDestroy();

		void AddComponent(std::shared_ptr<Component> component);

		void AddComponents(const std::initializer_list<std::shared_ptr<Component>>& newComponents);

		template <typename T>
		std::enable_if_t<std::is_base_of<Component, T>::value, T*> GetComponent()
		{
			T* result = nullptr;
			for (auto c : components)
			{
				result = dynamic_cast<T*>(c.get());
				if (result)
					return result;
			}
			return result;
		}

		template <typename T>
		std::enable_if_t<std::is_base_of<Component, T>::value, std::vector<T*>> GetComponents()
		{
			std::vector<T*> result;
			for (auto c : components)
			{
				auto item = dynamic_cast<T*>(c.get());
				if (item)
				{
					result.push_back(item);
				}
			}
			return result;
		}

		std::shared_ptr<Transform> transform = std::make_shared<Transform>(Transform(this));
		std::vector<std::shared_ptr<Component>> components;
		std::string name;
	};

}