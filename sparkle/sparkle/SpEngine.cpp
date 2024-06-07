#include "SpEngine.h"

#include <algorithm>

#include <fmt/core.h>
#include <fmt/color.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#include "stb_image.h"

#include "Scene.h"
#include "Global.h"
#include "GameInstance.h"
#include "Resource.h"
#include "Input.h"
#include "GUI.h"

namespace sparkle
{
	void PrintGlfwError(int error, const char* description)
	{
		fmt::print("Error(Glfw): Code({}), {}\n", error, description);
	}

	SpEngine::SpEngine()
	{
		Global::engine = this;
		// setup glfw
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		// Multi-sample anti-aliasing
		glfwWindowHint(GLFW_SAMPLES, 4);

		m_window = glfwCreateWindow(Global::Config::screenWidth, Global::Config::screenHeight, "Sparkle", NULL, NULL);

		if (m_window == NULL)
		{
			fmt::print("Failed to create GLFW window\n");
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(0);

		glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* m_window, int width, int height) {
			glViewport(0, 0, width, height);
			});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* m_window, double xpos, double ypos) {
			Global::game->ProcessMouse(m_window, xpos, ypos);
			});

		glfwSetScrollCallback(m_window, [](GLFWwindow* m_window, double xoffset, double yoffset) {
			Global::game->ProcessScroll(m_window, xoffset, yoffset);
			});

		glfwSetErrorCallback(PrintGlfwError);

		// setup opengl
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			fmt::print("Failed to initialize GLAD\n");
			return;
		}
		glViewport(0, 0, Global::Config::screenWidth, Global::Config::screenHeight);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// setup stbi
		stbi_set_flip_vertically_on_load(true);

		// setup members
		m_gui = std::make_shared<GUI>(m_window);
		m_input = std::make_shared<Input>(m_window);
	}

	SpEngine::~SpEngine()
	{
		m_gui->ShutDown();
		glfwTerminate();
	}

	void SpEngine::SetScenes(const std::vector<std::shared_ptr<Scene>>& initializers)
	{
		scenes = initializers;
	}

	int SpEngine::Run()
	{
		do {
			fmt::print("Sparkle Engine\n");

			m_game = std::make_shared<GameInstance>(m_window, m_gui);
			sceneIndex = m_nextSceneIndex;
			// TODO Note we're always repopulating each scene from disk, including re-compiling shaders.
			// This is fine for now, but we should consider caching the compiled shaders (and assets...)
			scenes[sceneIndex]->PopulateActors(m_game.get());
			scenes[sceneIndex]->onEnter.Invoke();
			m_game->Run();
			scenes[sceneIndex]->onExit.Invoke();
			scenes[sceneIndex]->ClearCallbacks();

			Resource::ClearCache();
			m_gui->ClearCallback();
		} while (m_game->pendingReset);

		return 0;
	}

	void SpEngine::Reset()
	{
		m_game->pendingReset = true;
	}

	void SpEngine::SwitchScene(unsigned int _sceneIndex)
	{
		m_nextSceneIndex = std::clamp(_sceneIndex, 0u, (unsigned int)scenes.size() - 1);
		m_game->pendingReset = true;
	}

	glm::ivec2 SpEngine::windowSize()
	{
		glm::ivec2 result;
		glfwGetWindowSize(m_window, &result.x, &result.y);
		return result;
	}
}