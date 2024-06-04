#pragma once

#include "fmt/format.h"
#include "glm.hpp"

template <>
struct fmt::formatter<glm::vec3> : fmt::formatter<std::string> {
	auto format(glm::vec3 p, format_context& ctx) {
		return formatter<std::string>::format(
			fmt::format("[{:.2f}, {:.2f}, {:.2f}]", p.x, p.y, p.z), ctx);
	}
};

template <>
struct fmt::formatter<glm::vec2> : fmt::formatter<std::string> {
	auto format(glm::vec2 p, format_context& ctx) {
		return formatter<std::string>::format(
			fmt::format("[{:.2f}, {:.2f}]", p.x, p.y), ctx);
	}
};

namespace sparkle
{
	namespace utils
	{
		glm::mat4 RotateEuler(glm::mat4 result, const glm::vec3& rotation);

		glm::vec3 RotateEuler(glm::vec3 result, const glm::vec3& rotation);

		float Random(float min, float max);

		template <class T>
		T Lerp(T value1, T value2, float a)
		{
			a = std::min(std::max(a, 0.0f), 1.0f);
			return a * value2 + (1 - a) * value1;
		}
	}
}