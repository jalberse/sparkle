#pragma once

#include <functional>

namespace sparkle
{
	// Allows objects with the same material to have different properties.
	struct MaterialProperty
	{
		std::function<void(Material*)> preRendering;
	};
}