#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#include "Component.h"
#include "Common.h"

namespace sparkle
{

	class Light;
	class RenderPipeline;
	class GUI;
	class Actor;
	class Timer;

	class GameInstance
	{
	public:
		GameInstance(GLFWwindow* window, std::shared_ptr<GUI> gui);
		GameInstance(const GameInstance&) = delete;

		std::shared_ptr<Actor> AddActor(std::shared_ptr<Actor> actor);
		std::shared_ptr<Actor> CreateActor(const std::string& name);

		int Run();

		void ProcessMouse(GLFWwindow* window, double xpos, double ypos);
		void ProcessScroll(GLFWwindow* window, double xoffset, double yoffset);
		void ProcessKeyboard(GLFWwindow* window);

		template <typename T>
		std::enable_if_t<std::is_base_of<Component, T>::value, std::vector<T*>> FindComponents()
		{
			std::vector<T*> result;
			for (auto actor : m_actors)
			{
				// TODO This throws errors when compiled aginst C++20 standard, unsure why?
				// In 20, it would be better to convert this fn (and the Actor fn) as concepts/constraints.
				// Maybe we should make that upgrade (I do love concepts/constraints...) but for now let's
				// compile with C++17, and we can do that upgrade later.
				auto component = actor->GetComponents<T>();
				if (component.size() > 0)
				{
					result.insert(result.end(), component.begin(), component.end());
				}
			}
			return result;
		}

		unsigned int depthFrameBuffer();
		unsigned int depthTex();
		glm::ivec2 windowSize();
		bool windowMinimized();

		SpCallback<void(double, double)> onMouseScroll;
		SpCallback<void(double, double)> onMouseMove;
		SpCallback<void()> animationUpdate;
		SpCallback<void()> godUpdate; // update when main logic is paused (for debugging purposes)
		SpCallback<void()> onFinalize;

		bool pendingReset = false;
		glm::vec4 clearColor = glm::vec4(0.0f);

	private:
		void Initialize();
		void MainLoop();
		void Finalize();

		GLFWwindow* m_window = nullptr;
		std::shared_ptr<GUI> m_gui;
		std::shared_ptr<Timer> m_timer;

		std::vector<std::shared_ptr<Actor>> m_actors;
		std::shared_ptr<RenderPipeline> m_renderPipeline;
	};
}