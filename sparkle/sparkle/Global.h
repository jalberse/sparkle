#pragma once

#include <vector>

#include "Common.h"

namespace sparkle
{
	class Light;
	class GameInstance;
	class Input;
	class SpEngine;
	class Camera;

	namespace Global
	{
		inline SpEngine* engine;
		inline GameInstance* game;
		inline Camera* camera;
		inline Input* input;
		inline std::vector<Light*> lights;

		inline SpGameState gameState;
		inline SpSimParams simParams;

		namespace Config
		{
			// Controls how fast the camera moves
			const float cameraTranslateSpeed = 5.0f;
			const float cameraRotateSensitivity = 0.15f;

			const unsigned int screenWidth = 1600;
			const unsigned int screenHeight = 900;

			const unsigned int shadowWidth = 1024;
			const unsigned int shadowHeight = 1024;
		}
	}
}