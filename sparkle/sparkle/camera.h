#pragma once


#include <glad/glad.h>
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "Component.h"
#include "Global.h"
#include "Actor.h"
#include "GameInstance.h"
#include "utils.h"

namespace sparkle
{
	class Camera : public Component
	{
	public:
		float zoom = 45.0f;

		Camera()
		{
			SET_COMPONENT_NAME
			Global::camera = this;
		}

		glm::vec3 position() const
		{
			return actor->transform->position;
		}

		glm::vec3 front() const
		{
			const glm::vec3 kFront = glm::vec3(0.0f, 0.0f, -1.0f);
			return utils::RotateEuler(kFront, actor->transform->rotation);
		}

		glm::vec3 up() const
		{
			const glm::vec3 kUp = glm::vec3(0.0f, 1.0f, 0.0f);
			return utils::RotateEuler(kUp, actor->transform->rotation);
		}

		glm::mat4 view() const
		{
			return glm::lookAt(position(), position() + front(), up());
		}

		glm::mat4 projection() const
		{
			auto size = Global::game->windowSize();
			auto screenAspect = (float)size.x / (float)size.y;
			return glm::perspective(glm::radians(zoom), screenAspect, 0.01f, 100.0f);
		}
	};
}
