#include "GameInstance.h"

#include <memory>

#include "Global.h"
#include "RenderPipeline.h"
#include "Actor.h"
#include "Input.h"
#include "SpEngine.h"
#include "Timer.h"
#include "GUI.h"

namespace sparkle
{
	GameInstance::GameInstance(GLFWwindow* window, std::shared_ptr<GUI> gui)
	{
		Global::game = this;

		m_window = window;
		m_gui = gui;
		m_renderPipeline = std::make_shared<RenderPipeline>();
		m_timer = std::make_shared<Timer>();

		Timer::StartTimer("GAME_INSTANCE_INIT");
	}

	std::shared_ptr<Actor> GameInstance::AddActor(std::shared_ptr<Actor> actor)
	{
		m_actors.push_back(actor);
		return actor;
	}

	std::shared_ptr<Actor> GameInstance::CreateActor(const std::string& name)
	{
		auto actor = std::make_shared<Actor>(name);
		return AddActor(actor);
	}

	int GameInstance::Run()
	{
		Initialize();
		MainLoop();
		Finalize();

		return 0;
	}

	unsigned int GameInstance::depthFrameBuffer()
	{
		return m_renderPipeline->depthFrameBuffer;
	}

	unsigned int GameInstance::depthTex()
	{
		return m_renderPipeline->depthTex;
	}

	glm::ivec2 GameInstance::windowSize()
	{
		glm::ivec2 result;
		glfwGetWindowSize(m_window, &result.x, &result.y);
		return result;
	}

	bool GameInstance::windowMinimized()
	{
		auto size = windowSize();
		return size.x == 0 || size.y == 0;
	}

	void GameInstance::ProcessMouse(GLFWwindow* window, double xpos, double ypos)
	{
		onMouseMove.Invoke(xpos, ypos);
	}

	void GameInstance::ProcessScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		onMouseScroll.Invoke(xoffset, yoffset);
	}

	void GameInstance::ProcessKeyboard(GLFWwindow* window)
	{
		Global::input->ToggleOnKeyDown(GLFW_KEY_H, Global::gameState.hideGUI);

		if (Global::input->GetKey(GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(m_window, true);
		}
		if (Global::input->GetKey(GLFW_KEY_O))
		{
			Global::gameState.step = true;
			Global::gameState.pause = false;
		}
		for (int i = 0; i < 9; i++)
		{
			if (Global::input->GetKeyDown(GLFW_KEY_1 + i))
			{
				Global::engine->SwitchScene(i);
			}
		}
		if (Global::input->GetKeyDown(GLFW_KEY_R))
		{
			Global::engine->Reset();
		}
	}

	void GameInstance::Initialize()
	{
		for (const auto& go : m_actors)
		{
			go->Start();
		}
	}

	void GameInstance::MainLoop()
	{
		double initTime = Timer::EndTimer("GAME_INSTANCE_INIT") * 1000.0;
		fmt::print("Info(GameInstance): Initialization success within {:.2f} ms. Enter main loop.\n", initTime);
		// render loop
		while (!glfwWindowShouldClose(m_window) && !pendingReset)
		{
			if (windowMinimized())
			{
				glfwPollEvents();
				continue;
			}
			// Input
			ProcessKeyboard(m_window);

			// Init
			glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glPolygonMode(GL_FRONT_AND_BACK, Global::gameState.renderWireframe ? GL_LINE : GL_FILL);

			Timer::StartTimer("CPU_TIME");
			Timer::UpdateDeltaTime();

			// Logic Updates
			if (!Global::gameState.hideGUI)
			{
				m_gui->OnUpdate();
			}

			if (!Global::gameState.pause)
			{
				Timer::NextFrame();
				if (Timer::NextFixedFrame())
				{
					for (const auto& go : m_actors)
					{
						go->FixedUpdate();
					}

					animationUpdate.Invoke();

					if (Global::gameState.step)
					{
						Global::gameState.pause = true;
						Global::gameState.step = false;
					}
				}

				for (const auto& go : m_actors)
				{
					go->Update();
				}
			}

			Global::input->OnUpdate();

			godUpdate.Invoke();

			Timer::EndTimer("CPU_TIME");

			// Render
			m_renderPipeline->Render();
			if (!Global::gameState.hideGUI)
			{
				m_gui->Render();
			}

			// Check and call events and swap the buffers
			glfwSwapBuffers(m_window);
			glfwPollEvents();
		}
	}

	void GameInstance::Finalize()
	{
		for (const auto& go : m_actors)
		{
			go->OnDestroy();
		}
	}
}