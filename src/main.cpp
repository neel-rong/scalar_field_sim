#include "glfw_window.h"
#include "GUI.h"
#include "simulation.h"
#include "renderer.h"
#include "sim_properties.h"
#include "data_type.h"
#include "field.h"
#include "simulation_operators.h"


int main()
{
	// Initialize GLFW Window
	Window m_window(1280, 720, "Scalar Field Simulation");

	// Profiler
	FrameProfiler m_frameProfiler;

	// Initialize Domain Config
	DomainConfig m_domainConfig;

	// Initialize Simulation Config
	SimulationConfig m_simulationConfig;
	m_simulationConfig.Reset();
	m_simulationConfig.EmitterProperty.Reset(m_domainConfig);

	// Initialize Render Config
	RenderConfig m_renderConfig;

	//Mouse State
	MouseState mouse_state;

	// Initialize Playback
	Playback playback = Playback::Stop;

	// Initialize Simulation
	Simulation m_simulation(&m_domainConfig, &m_simulationConfig, &m_renderConfig, &m_frameProfiler, &mouse_state, &playback);
	m_simulation.Init();

	// Initialize Renderer
	Renderer m_renderer(&m_renderConfig, &m_simulationConfig, m_simulation.m_renderFieldScalar, m_simulation.m_renderFieldVelocity);
	m_renderer.Init();

	// Initialize GUI
	GUI gui(&m_domainConfig, &m_simulationConfig, &m_renderConfig, &playback, &m_frameProfiler, m_renderer.GetTexID(), m_window.GetNativeWindow());
	gui.Initialize();



	while (!m_window.ShouldClose())
	{
		m_window.Update(m_frameProfiler);

		m_renderer.UpdateFields(m_simulation.m_renderFieldScalar, m_simulation.m_renderFieldVelocity);

		// Draw Gui
		gui.BeginFrame();

		gui.Draw();

		m_renderer.BeginFrame();

		switch (playback)
		{
			case Playback::Pause :
			{
				break;
			}

			case Playback::Play :
			{
				gui.UpdateMouseState();
				gui.GetMouseState(mouse_state);

				m_simulation.Update();

				m_simulation.RunSimulation();
				m_renderer.Update(m_simulation.m_pixelValueMin, m_simulation.m_pixelValueMax, m_simulation.m_velocityMin, m_simulation.m_velocityMax);

				break;
			}

			case Playback::Reset :
			{
				playback = Playback::Stop;
				m_simulation.Init();
				m_renderer.Reset();
				m_simulationConfig.EmitterProperty.Reset(m_domainConfig);
				break;
			}

			case Playback::Stop:
			{
				m_simulation.Reset();
				break;
			}

		}

		m_renderer.DrawFrame();

		gui.EndFrame();

		m_window.SwapBuffers();
		m_window.PollEvents();

	}

	return 0;
}