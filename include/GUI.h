#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include "sim_properties.h"
#include "debug_helpers.h"
#include "misc_operators.h"


class GUI
{
public:

	// Constructors
	GUI() : m_simulationConfig(nullptr), m_renderConfig(nullptr), m_window(nullptr) {}

	GUI(DomainConfig* InDomainConfig, SimulationConfig* InSimulationConfig, RenderConfig* InRenderConfig, Playback* InPlayback, FrameProfiler* InFrameProfiler, GLuint InTexID, GLFWwindow* InWindow) :
		m_domainConfig(InDomainConfig), m_simulationConfig(InSimulationConfig), m_renderConfig(InRenderConfig), m_playback(InPlayback), m_frameProfiler(InFrameProfiler), tex_id(InTexID), m_window(InWindow) {}

	// Destructor
	~GUI() = default;

	// Initialize the UI
	void Initialize();

	// Rendering functions
	void BeginFrame();
	void Draw();
	void EndFrame();


	// Input processing function
	void SetSimulationConfigs(SimulationConfig* Configs);
	void SetRenderConfigs(RenderConfig* Configs);

	// Mouse State Functions
	void GetMouseState(MouseState& OutMouseState);
	void UpdateMouseState();

	// Docking Panels

	void InitializeDocking(ImGuiID InDockSpaceID);
	void DrawEditorPanel();
	void DrawViewportPanel();
	void DrawPlaybarPanel();
	void UpdateViewport(GLuint InTexID);
	void ShowToolTip(const char* Label, const char* Description);

	void DrawValue(const char* Label, float* Value)
	{
		ImGui::InputFloat(Label, Value);
	}

	void DrawValue(const char* Label, Float2* Value)
	{
		ImGui::InputFloat2(Label, &Value->x);
	}


	template<ArithmeticField T>
	void DrawEmitterProperties(const char* Title, EmitterProperties<T>& EmitterProperties, const std::string& InLabel, const std::string& InDescription)
	{
		ImGui::SeparatorText(Title);

		ImGui::PushID(Title);

		ShowToolTip(
			InLabel.c_str(),
			InDescription.c_str()
		);

		ImGui::InputFloat(
			"Injection Radius",
			&EmitterProperties.Radius
		);

		ShowToolTip("Injection Radius", "The radius of the injection area for the emitter. This defines the size of the area where the emitter will inject values into the simulation.");

		ImGui::SliderFloat(
			"Injection Position",
			&EmitterProperties.Position.x,
			0,
			m_domainConfig->DomainSize.x
		);

		ShowToolTip("Injection Position", "The position of the injection area for the emitter. This defines the center of the area where the emitter will inject values into the simulation.");

		DrawValue("Injection Value Average", &EmitterProperties.InjectionValueAverage);

		ShowToolTip("Injection Value Average", "The average value that the emitter will inject into the simulation. This defines the mean of the values that will be injected.");

		DrawValue("Injection Value Variance", &EmitterProperties.InjectionValueVariance);

		ShowToolTip("Injection Value Variance", "The variance of the values that the emitter will inject into the simulation. This defines the spread of the values that will be injected.");

		ImGui::PopID();
	}


	// Helper Functions

private:

	// GUI State
	MouseState m_mouseState;

	// Shared State Variables
	SimulationConfig* m_simulationConfig = nullptr;
	RenderConfig* m_renderConfig = nullptr;
	Playback* m_playback = nullptr;
	FrameProfiler* m_frameProfiler = nullptr;
	DomainConfig* m_domainConfig = nullptr;

	// Docking state
	bool bDockInitialized = false;

	// GLFW window pointer
	GLFWwindow* m_window = nullptr;

	// Texture Parameters
	GLuint tex_id = 0;
	ImVec2 m_imageMin = ImVec2(0, 1);
	ImVec2 m_imageMax = ImVec2(1, 0);
	ImVec2 m_imageSize = ImVec2(0, 0);
};