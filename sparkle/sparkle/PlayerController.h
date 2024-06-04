#pragma once

#include <memory>
#include <functional>

#include "Global.h"
#include "Component.h"
#include "GameInstance.h"
#include "Camera.h"
#include "Input.h"
#include "Timer.h"

#include "fmt/core.h"


namespace sparkle
{
	class PlayerController : public Component
	{
	public:
		PlayerController()
		{
			SET_COMPONENT_NAME
		}

		void Start() override
		{
			Global::game->onMouseScroll.Register(OnMouseScroll);
			Global::game->onMouseMove.Register(OnMouseMove);
			Global::game->godUpdate.Register(GodUpdate);
		}

		static void GodUpdate()
		{
			const auto& camera = Global::camera;

			if (camera)
			{
				const auto& trans = camera->transform();
				const float speedScalar = Global::Config::cameraTranslateSpeed;

				static glm::vec3 currentVelocity(0);
				glm::vec3 targetVelocity(0);

				if (Global::input->GetKey(GLFW_KEY_W))
				{
					targetVelocity += camera->front();
				} 
				else if (Global::input->GetKey(GLFW_KEY_S))
				{
					targetVelocity -= camera->front();
				}

				if (Global::input->GetKey(GLFW_KEY_A))
				{
					targetVelocity -= glm::normalize(glm::cross(camera->front(), camera->up()));
				}
				else if (Global::input->GetKey(GLFW_KEY_D))
				{
					targetVelocity += glm::normalize(glm::cross(camera->front(), camera->up()));
				}

				if (Global::input->GetKey(GLFW_KEY_Q))
				{
					targetVelocity += camera->up();
				}
				else if (Global::input->GetKey(GLFW_KEY_E))
				{
					targetVelocity -= camera->up();
				}

				currentVelocity = utils::Lerp(currentVelocity, targetVelocity, Timer::deltaTime() * 10);
				trans->position += currentVelocity * speedScalar * Timer::deltaTime();
			}
			else
			{
				fmt::print("Error: Camera not found.\n");
			}
		}

		static void OnMouseScroll(double xoffset, double yoffset)
		{
			auto camera = Global::camera;
			camera->zoom -= (float)yoffset;
			if (camera->zoom < 1.0f)
			{
				camera->zoom = 1.0f;
			}
			if (camera->zoom > 45.0f)
			{
				camera->zoom = 45.0f;
			}
		}

		static void OnMouseMove(double xpos, double ypos)
		{
			static float lastX = Global::Config::screenWidth / 2.0f;
			static float lastY = Global::Config::screenHeight / 2.0f;

			bool shouldRotate = Global::input->GetMouse(GLFW_MOUSE_BUTTON_RIGHT);

			if (shouldRotate)
			{
				auto rot = Global::camera->transform()->rotation;
				float yaw = -rot.y;
				float pitch = rot.x;

				float xoffset = (float)xpos - lastX;
				float yoffset = lastY - (float)ypos;
				xoffset *= Global::Config::cameraRotateSensitivity;
				yoffset *= Global::Config::cameraRotateSensitivity;
				yaw += xoffset;
				pitch = std::clamp(pitch + yoffset, -89.0f, 89.0f);

				Global::camera->transform()->rotation = glm::vec3(pitch, -yaw, 0.0f);
			}
			lastX = (float)xpos;
			lastY = (float)ypos;
		}
	};
}