#include "Component.h"

#include "Actor.h"

namespace sparkle
{
	std::shared_ptr<Transform> Component::transform()
	{
		if (actor)
		{
			return actor->transform;
		}
		else
		{
			return std::make_shared<Transform>(Transform(nullptr));
		}
	}
}