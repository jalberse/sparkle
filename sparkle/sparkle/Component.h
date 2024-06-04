#pragma once

#include <iostream>
#include <string>

#include "Transform.h"

// __func__ returns the name of the current function; we can use it
// in the constructor to set the name to the current class name.
#define SET_COMPONENT_NAME name = __func__;

namespace sparkle
{
	class Actor;

	class Component
	{
	public:
		virtual void Start() {}

		virtual void Update() {}

		virtual void FixedUpdate() {}

		virtual void OnDestroy() {}

		std::string name = "Component";

		// TODO Instead, use std::optional<std::shared_ptr<Transform>>.
		Actor* actor = nullptr;

		std::shared_ptr<Transform> transform();

		bool enabled = true;
	};

}