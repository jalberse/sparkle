#pragma once

#include <glm.hpp>
#include <imgui.h>
#include <functional>
#include <vector>

#define IMGUI_LEFT_LABEL(func, label, ...) (ImGui::TextUnformatted(label), ImGui::SameLine(), func("##" label, __VA_ARGS__))

// Only initialize value on host. 
// Since CUDA doesn't allow dynamics initialization, 
// we use this macro to ignore initialization when compiling with NVCC.
#ifdef __CUDA_ARCH__
#define HOST_INIT(val) 
#else
#define HOST_INIT(val) = val
#endif

struct SpSimParams
{
	int numSubsteps HOST_INIT(2);
	int numIterations HOST_INIT(4); //!< Number of solver iterations to perform per-substep
	int maxNumNeighbors HOST_INIT(64);
	float maxSpeed HOST_INIT(50); //!< The magnitude of particle velocity will be clamped to this value at the end of each step

	// forces
	glm::vec3 gravity HOST_INIT(glm::vec3(0, -9.8f, 0));
	float bendCompliance HOST_INIT(10.0f);
	float damping HOST_INIT(0.25f);
	float relaxationFactor HOST_INIT(1.0f);
	float longRangeStretchiness HOST_INIT(1.2f);

	// collision
	float collisionMargin HOST_INIT(0.06f);
	float friction HOST_INIT(0.1f);
	bool enableSelfCollision HOST_INIT(true);
	int interleavedHash HOST_INIT(3);

	// runtime info
	unsigned int numParticles;
	float particleDiameter;
	float deltaTime;

	// misc
	float particleDiameterScalaer HOST_INIT(1.5f);
	float hashCellSizeScalar HOST_INIT(1.5f);

	// TODO - add more, e.g. float wind[3] or int relaxationMode, and other options to configure.

	void OnGUI()
	{
		IMGUI_LEFT_LABEL(ImGui::SliderInt, "Num Substeps", &numSubsteps, 1, 20);
		IMGUI_LEFT_LABEL(ImGui::SliderInt, "Num Iterations", &numIterations, 1, 20);
		IMGUI_LEFT_LABEL(ImGui::SliderFloat, "Max Speed", &maxSpeed, 1e-2f, 100);
		ImGui::Separator();
		IMGUI_LEFT_LABEL(ImGui::SliderFloat3, "Gravity", (float*)&gravity, -50, 50);
		IMGUI_LEFT_LABEL(ImGui::SliderFloat, "Friction", &friction, 0, 1);
		IMGUI_LEFT_LABEL(ImGui::SliderFloat, "Collision Margin", &collisionMargin, 0, 0.5);
		IMGUI_LEFT_LABEL(ImGui::Checkbox, "Enable Self Collision", &enableSelfCollision);
		IMGUI_LEFT_LABEL(ImGui::SliderInt, "Interleaved Hash", &interleavedHash, 1, 10);
		ImGui::Separator();
		IMGUI_LEFT_LABEL(ImGui::SliderFloat, "Relaxation Factor", &relaxationFactor, 0, 3.0);
		IMGUI_LEFT_LABEL(ImGui::SliderFloat, "Long Range Stretch", &longRangeStretchiness, 1.0, 2.0, "%.3f");

	}
};

struct SpGameState
{
	bool step = false;
	bool pause = false;
	bool renderWireframe = false;
	bool drawParticles = false;
	bool hideGUI = false;
	bool detailTimer = false;
};

template <class T, class... TArgs>
class SpCallback
{
public:
	void Register(const std::function<T>& func)
	{
		m_funcs.push_back(func);
	}

	template <class... TArgs>
	void Invoke(TArgs... args)
	{
		for (const auto& func : m_funcs)
		{
			func(std::forward<TArgs>(args)...);
		}
	}

	void Clear()
	{
		m_funcs.clear();
	}

	bool empty()
	{
		return m_funcs.size() == 0;
	}

private:
	std::vector<std::function<T>> m_funcs;
};
