#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

namespace sparkle
{
	class Scene;
	class GUI;
	class GameInstance;
	class Input;

	class SpEngine
	{
	public:
		SpEngine();
		~SpEngine();

		int Run();

		void Reset();
		void SwitchScene(unsigned int sceneIndex);
		void SetScenes(const std::vector<std::shared_ptr<Scene>>& scenes);

		glm::ivec2 windowSize();

		std::vector<std::shared_ptr<Scene>> scenes;
		unsigned int sceneIndex = 0;
	private:
		unsigned int m_nextSceneIndex = 0;
		GLFWwindow* m_window = nullptr;
		std::shared_ptr<GUI> m_gui;
		std::unique_ptr<GameInstance> m_game;
		std::unique_ptr<Input> m_input;
	};
}