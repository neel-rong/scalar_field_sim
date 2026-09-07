#pragma once
#include "GUI.h"


// =========================================================
// GUI class implementation
// =========================================================

// Initialize the GUI
void GUI::Initialize()
{
	// Initialize Imgui Components for the GUI
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_window, true);

	ImGui_ImplOpenGL3_Init("#version 330");

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}


// =========================================================
// Rendering functions
// =========================================================

// Begin a new frame for the GUI
void GUI::BeginFrame()
{
	// Start a new frame for the GUI
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
}

void GUI::Draw()
{
	// Draw the GUI elements

	// Docking Panels
	ImGuiID dockspace_id = ImGui::DockSpaceOverViewport();

	// Initialize the docking layout once after the dockspace has been created.
	if (!bDockInitialized)
	{
		InitializeDocking(dockspace_id);
		bDockInitialized = true;
	}


	DrawEditorPanel();
	DrawViewportPanel();
	DrawPlaybarPanel();

}

void GUI::EndFrame()
{
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// =========================================================
// Input processing functions
// =========================================================

void GUI::SetSimulationConfigs(SimulationConfig* Configs)
{
	m_simulationConfig = Configs;
}

void GUI::SetRenderConfigs(RenderConfig* Configs)
{
	m_renderConfig = Configs;
}


// =========================================================
// Mouse State Functions
// =========================================================

void GUI::GetMouseState(MouseState& OutMouseState)
{
	OutMouseState = m_mouseState;
}

void GUI::UpdateMouseState()
{
	// Get the current mouse position
	double mouseX, mouseY;
	glfwGetCursorPos(m_window, &mouseX, &mouseY);

	// Update the mouse state
	m_mouseState.PreviousWindowPosition = m_mouseState.WindowPosition;
	m_mouseState.WindowPosition = Double2(mouseX, mouseY);
	m_mouseState.DeltaWindow = m_mouseState.WindowPosition - m_mouseState.PreviousWindowPosition;

	m_mouseState.PreviousGridPosition = m_mouseState.GridPosition;

	if (static_cast<float>(m_mouseState.WindowPosition.x) >= m_imageMin.x && static_cast<float>(m_mouseState.WindowPosition.y) >= m_imageMin.y &&
		static_cast<float>(m_mouseState.WindowPosition.x) <= m_imageMax.x && static_cast<float>(m_mouseState.WindowPosition.y) <= m_imageMax.y)
	{
		// Calculate the normalized mouse position within the image
		// Note: The y-coordinate is inverted because the image origin is at the top-left corner as per ImGui's coordinate system,
		// while the mouse position is in window coordinates.

		double normalizedX = (mouseX - m_imageMin.x) / (m_imageMax.x - m_imageMin.x);
		double normalizedY = 1.0 - (mouseY - m_imageMin.y) / (m_imageMax.y - m_imageMin.y);

		// Map the normalized position to the grid resolution
		m_mouseState.GridPosition.x = normalizedX * m_domainConfig->GridResolution.x + 1;
		m_mouseState.GridPosition.y = normalizedY * m_domainConfig->GridResolution.y + 1;

		m_mouseState.DeltaGrid = m_mouseState.GridPosition - m_mouseState.PreviousGridPosition;
	}
	else
	{
		m_mouseState.GridPosition = Double2(-1.0, -1.0);	// Outside the image bounds
		m_mouseState.DeltaGrid = Double2(0.0, 0.0);			// Set delta to zero, if mouse out of grid bounds
	}

	Double2 delta = m_mouseState.DeltaGrid;

	if (m_frameProfiler->DeltaTime > 0.0)
	{
		m_mouseState.MouseVelocity = Double2(delta.x / m_frameProfiler->DeltaTime, delta.y / m_frameProfiler->DeltaTime);
	}
	else
	{
		m_mouseState.MouseVelocity = Double2(0.0, 0.0);
	}

	m_mouseState.MouseSteps = Int2(std::abs(static_cast<int>(delta.x)), std::abs(static_cast<int>(delta.y)));

	// Update button states
	m_mouseState.bLeftButtonDown = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	m_mouseState.bRightButtonDown = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}

// =========================================================
// Docking Panel Functions
// =========================================================

void GUI::InitializeDocking(ImGuiID InDockSpaceID)
{
	// Create dock space
	ImGui::DockBuilderRemoveNode(InDockSpaceID);
	ImGui::DockBuilderAddNode(InDockSpaceID, ImGuiDockNodeFlags_DockSpace);

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::DockBuilderSetNodeSize(InDockSpaceID, viewport->WorkSize);

	ImGuiID editor_dock;
	ImGuiID viewport_dock;
	ImGuiID playbar_dock;

	ImGui::DockBuilderSplitNode(
		InDockSpaceID,
		ImGuiDir_Down,
		0.075,
		&playbar_dock,
		&viewport_dock);

	ImGui::DockBuilderSplitNode(
		viewport_dock,
		ImGuiDir_Left,
		.2,
		&editor_dock,
		&viewport_dock);

	//Assign Panels
	ImGui::DockBuilderDockWindow("Editor", editor_dock);
	ImGui::DockBuilderDockWindow("Viewport", viewport_dock);
	ImGui::DockBuilderDockWindow("Playbar", playbar_dock);

	ImGui::DockBuilderFinish(InDockSpaceID);
}

void GUI::DrawEditorPanel()
{
	ImGui::Begin("Editor");


	// Create Collapse Headers

	// Simulation Settings
	ImGui::SeparatorText("Simulation Settings");
	if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputFloat(
			"Time Step",
			&m_simulationConfig->TimeStep
		);

		ShowToolTip(
			"Time Step",
			"Time step for the simulation. Smaller values will result in more accurate simulations, but will also be slower."
		);

		ImGui::InputFloat(
			"Advection Scale",
			&m_simulationConfig->AdvectionScale
		);

		ShowToolTip(
			"Advection Scale",
			"Scale factor for the advection term in the simulation. Higher values will result in more pronounced advection effects."
		);

		ImGui::InputFloat(
			"Velocity Mouse Injection Scale",
			&m_simulationConfig->VelocityMouseInjectionScale
		);

		ShowToolTip(
			"Velocity Mouse Injection Scale",
			"Scale factor for the velocity injected by the mouse. Higher values will result in more pronounced velocity effects."
		);

		ImGui::InputFloat(
			"Mouse Injection Radius",
			&m_simulationConfig->MouseInjectionRadius
		);

		ShowToolTip(
			"Mouse Injection Radius",
			"Radius of the area affected by the mouse injection. Larger values will result in a larger area being affected."
		);

		int SMode = static_cast<int>(m_simulationConfig->SimulationMode);
		const char* SimulationMode[2];
		SimulationConfig::GetSimulationModeString(SimulationMode, sizeof(SimulationMode));
		if (ImGui::Combo(
			"Simulation Mode",
			&SMode,
			SimulationMode,
			IM_ARRAYSIZE(SimulationMode)
		))
		{
			m_simulationConfig->SimulationMode = static_cast<SimulationConfig::SimulationModeEnum>(SMode);
		}

		ShowToolTip(
			"Simulation Mode",
			"Select the simulation mode. Test mode is for testing the simulation, while Simulation mode is for running the simulation."
		);

		if (m_simulationConfig->SimulationMode == SimulationConfig::SimulationModeEnum::Test)
		{
			int TMode = static_cast<int>(m_simulationConfig->TestMode);
			const char* TestMode[2];
			SimulationConfig::GetTestModeString(TestMode, sizeof(TestMode));
			if (ImGui::Combo(
				"Test Mode",
				&TMode,
				TestMode,
				IM_ARRAYSIZE(TestMode)
			))
			{
				m_simulationConfig->TestMode = static_cast<SimulationConfig::TestModeEnum>(TMode);
			}

			ShowToolTip(
				"Test Mode",
				"Select the test mode. Divergence test is for testing the divergence of a constant velocity field, while Pressure test is for testing the pressure solver."
				"\nNote: Enable Reset Pressure for an stricter solver convergence test, when using Pressure test."
			);
		}

		ImGui::Checkbox(
			"Reset Pressure",
			&m_simulationConfig->bResetPressureField
		);

		ShowToolTip(
			"Reset Pressure Field Every Frame",
			"If checked, the pressure field will be reset every frame. This is useful for testing the simulation, but will result in less accurate simulations."
		);

		int PSolver = static_cast<int>(m_simulationConfig->PressureSolver);
		const char* PressureSolvers[3];
		SimulationConfig::GetPressureSolverString(PressureSolvers, sizeof(PressureSolvers));
		if (ImGui::Combo(
			"Pressure Solver",
			&PSolver,
			PressureSolvers,
			IM_ARRAYSIZE(PressureSolvers)
		))
		{
			m_simulationConfig->PressureSolver = static_cast<SimulationConfig::PressureSolverEnum>(PSolver);
		}

		ShowToolTip(
			"Pressure Solver",
			"Select the pressure solver to use for the simulation. Jacobi is the simplest solver, while Gauss-Seidel and Red-Black Gauss-Seidel are more advanced solvers that can converge faster."
		);


		ImGui::InputInt(
			"Pressure Iterations",
			&m_simulationConfig->SolverIterations
		);

		ShowToolTip(
			"Pressure Solver Iterations",
			"Number of iterations to perform for the pressure solver. More iterations will result in more accurate simulations, but will also be slower."
		);

		if (m_simulationConfig->PressureSolver == SimulationConfig::PressureSolverEnum::GaussSeidel ||
			m_simulationConfig->PressureSolver == SimulationConfig::PressureSolverEnum::RedBlackGaussSeidel)
		ImGui::InputFloat(
			"Over Relaxation",
			&m_simulationConfig->OverRelaxation
		);

		ShowToolTip(
			"Over Relaxation",
			"Over relaxation factor for the Gauss-Seidel and Red-Black Gauss-Seidel solvers. Higher values will result in faster convergence, but may also result in instability."
		);


		ImGui::InputInt2(
			"Grid Resolution",
			&m_domainConfig->GridResolution.x
		);

		ShowToolTip(
			"Grid Resolution",
			"Resolution of the simulation grid. Higher values will result in more accurate simulations, but will also be slower."
			"\n Changing the value of the grid resolution requires a reset of the simulation to take effect."
			"\nNote: The aspect ratio of the grid resolution should match the aspect ratio of the domain size."
		);

		ImGui::InputFloat2(
			"Domain Size",
			&m_domainConfig->DomainSize.x
		);

		ShowToolTip(
			"Domain Size",
			"Size of the simulation domain in physical units."
			"\n Changing the value of the grid resolution requires a reset of the simulation to take effect."
			"\nNote: The aspect ratio of the grid resolution should match the aspect ratio of the domain size."
		);
	}


	// Emitter Settings
	ImGui::SeparatorText("Emitter Settings");

	if (ImGui::CollapsingHeader("Emitter", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Density Emitter Settings
		DrawEmitterProperties("Density Emitter", m_simulationConfig->DensityEmitterProperties,
			"Density Emitter",
			"Settings for the density emitter. The density emitter injects density into the simulation at a specified position and radius."
			);


		// Velocity Emitter Settings
		DrawEmitterProperties("Velocity U Emitter", m_simulationConfig->VelocityUEmitterProperties,
			"Velocity U Emitter",
			"Settings for the velocity U emitter. The velocity U emitter injects velocity into the simulation at a specified position and radius."
			);

		DrawEmitterProperties("Velocity V Emitter", m_simulationConfig->VelocityVEmitterProperties,
			"Velocity V Emitter",
			"Settings for the velocity V emitter. The velocity V emitter injects velocity into the simulation at a specified position and radius."
			);
	}

	// Render Settings
	ImGui::SeparatorText("Render Settings");

	if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
	{

		int svMode = static_cast<int>(m_renderConfig->ScalarMode);
		const char* scalarDisplayModes[3];
		RenderConfig::GetScalarVisualizationModeString(scalarDisplayModes, sizeof(scalarDisplayModes));

		if (ImGui::Combo(
			"Visualization Mode",
			&svMode,
			scalarDisplayModes,
			IM_ARRAYSIZE(scalarDisplayModes)
		))
		{
			m_renderConfig->ScalarMode = static_cast<RenderConfig::ScalarVisualizationMode>(svMode);
		}

		ShowToolTip(
			"Scalar Visualization Mode",
			"Select the scalar visualization mode. This determines how scalar fields (like density or pressure) are visualized in the simulation."
		);

		int vvMode = static_cast<int>(m_renderConfig->VelocityMode);
		const char* velocityDisplayModes[3];
		RenderConfig::GetVelocityVisualizationModeString(velocityDisplayModes, 3);

		if (ImGui::Combo(
			"Velocity Visualization Mode",
			&vvMode,
			velocityDisplayModes,
			IM_ARRAYSIZE(velocityDisplayModes)
		))
		{
			m_renderConfig->VelocityMode = static_cast<RenderConfig::VelocityVisualizationMode>(vvMode);
		}

		ShowToolTip(
			"Velocity Visualization Mode",
			"Select the velocity visualization mode. This determines how velocity fields are visualized in the simulation."
		);

		ImGui::Checkbox(
			"Show Grid",
			&m_renderConfig->bShowGrid
		);

		ImGui::InputInt2(
			"Render Resolution",
			&m_renderConfig->RenderResolution.x
		);

		ShowToolTip(
			"Render Resolution",
			"Resolution of the rendered output. Higher values will result in more detailed visualizations, but will also be slower."
		);
	}

	ImGui::End();

}


void GUI::DrawViewportPanel()
{
	ImGui::Begin("Viewport");

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	float imageScale = std::min(viewportSize.x / m_renderConfig->RenderResolution.x, viewportSize.y / m_renderConfig->RenderResolution.y) * 0.95f;

	ImVec2 imageSize = ImVec2(m_renderConfig->RenderResolution.x * imageScale, m_renderConfig->RenderResolution.y * imageScale);

	ImVec2 CenterOffset = ImVec2((viewportSize.x - imageSize.x) * 0.5f, (viewportSize.y - imageSize.y) * 0.5f);

	ImGui::SetCursorPos(CenterOffset);

	ImGui::Image({ tex_id }, imageSize, ImVec2(0, 1), ImVec2(1, 0));

	m_imageMin = ImGui::GetItemRectMin();
	m_imageMax = ImGui::GetItemRectMax();
	m_imageSize = imageSize;

	// Display FPS and Frame Time

	ImGui::Text("FPS : %.1f", m_frameProfiler->FPS);
	ImGui::Text("Frame Time : %.1f", m_frameProfiler->DeltaTime * 1000.0f);

	ImGui::End();
}


void GUI::DrawPlaybarPanel()
{
	ImGui::Begin("Playbar");

	// Draw Playback Buttons
	auto DrawButton = [](const char* Label, Playback State, Playback* CurrentPlayback, ImVec4 ActiveColor, ImVec4 InactiveColor)
	{
		if (*CurrentPlayback == State)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor); // Green for active state
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, InactiveColor); // Gray for inactive state
		}

		if (ImGui::Button(Label, ImVec2(100, 25)))
		{
			*CurrentPlayback = State;
		}

		ImGui::PopStyleColor();
	};

	DrawButton("Play", Playback::Play, m_playback, ImVec4(0.0f, 0.5f, 0.1f, 1.0f), ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
	ImGui::SameLine();
	DrawButton("Pause", Playback::Pause, m_playback, ImVec4(0.6f, 0.4f, 0.0f, 1.0f), ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
	ImGui::SameLine();
	DrawButton("Stop", Playback::Stop, m_playback, ImVec4(0.5f, 0.0f, 0.0f, 1.0f), ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
	ImGui::SameLine();
	DrawButton("Reset", Playback::Reset, m_playback, ImVec4(0.0f, 0.0f, 0.5f, 1.0f), ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

	ImGui::End();
}


void GUI::UpdateViewport(GLuint InTexID)
{
	tex_id = InTexID;
}

void GUI::ShowToolTip(const char* Label, const char* Description)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%s", Label);
		ImGui::Separator();
		ImGui::Text("%s", Description);
		ImGui::EndTooltip();
	}
}
