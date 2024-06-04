#include "utils.h"

#include "gtc/matrix_transform.hpp"

namespace sparkle
{
	namespace utils
	{
		float Random(float min, float max)
		{
			float zeroToOne = (float)rand() / RAND_MAX;
			return min + zeroToOne * (max - min);
		}

		glm::mat4 RotateEuler(glm::mat4 result, const glm::vec3& rotation)
		{
			result = glm::rotate(result, glm::radians(rotation.y), glm::vec3(0, 1, 0));
			result = glm::rotate(result, glm::radians(rotation.z), glm::vec3(0, 0, 1));
			result = glm::rotate(result, glm::radians(rotation.x), glm::vec3(1, 0, 0));

			return result;
		}

		glm::vec3 RotateEuler(glm::vec3 result, const glm::vec3& rotation)
		{
			glm::mat4 rotationMatrix(1);
			rotationMatrix = RotateEuler(rotationMatrix, rotation);
			result = rotationMatrix * glm::vec4(result, 0.0f);
			return glm::normalize(result);
		}
	}
}