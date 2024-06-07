#pragma once

#include <functional>

namespace sparkle
{
	// Allows objects with the same material to have properties which differ from the defaults in the material.
	struct MaterialProperty
	{
		std::function<void(Material*)> preRendering;
	};
}