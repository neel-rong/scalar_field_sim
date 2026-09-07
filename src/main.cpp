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
	Window Window(1280, 720, "Scalar Field Simulation");

	// Profiler
	FrameProfiler frame_profiler;

	// Initialize Simulation Configs
	DomainConfig m_DomainConfig(Int2(128, 128), Float2(128, 128));

	// Initialize Dye Properties
	DyeProperties dye_properties(0.01f, 0.1f, 0.1f, 2.5f);

	// Initialize Emitter Properties
	EmitterProperties<Field2D<float>> densityEmitterProperties(Float2(m_DomainConfig.DomainSize.x / 2, m_DomainConfig.DomainSize.y / 6), 6.0f, 1.0f, 0.1f, .03f);
	EmitterProperties<Field2D<float>> velocityUEmitterProperties(Float2(m_DomainConfig.DomainSize.x / 2, m_DomainConfig.DomainSize.y / 6), 6.0f, 0.0f, 1.0f, .03f);
	EmitterProperties<Field2D<float>> velocityVEmitterProperties(Float2(m_DomainConfig.DomainSize.x / 2, m_DomainConfig.DomainSize.y / 6), 6.0f, 1.5f, 0.0f, .03f);

	// Initialize Simulation Configs
	SimulationConfig simulation_configs(densityEmitterProperties, velocityUEmitterProperties, velocityVEmitterProperties, dye_properties);

	// Initialize Render Configs
	RenderConfig m_renderConfig;

	//Mouse State
	MouseState mouse_state;

	// Initialize Playback
	Playback playback = Playback::Stop;

	// Initialize Simulation
	Simulation simulation(&m_DomainConfig, &simulation_configs, &m_renderConfig, &frame_profiler, &mouse_state, &playback);
	simulation.Init();
	simulation.Reset();

	// Initialize Renderer
	Renderer Renderer(&m_renderConfig, &simulation_configs, simulation.m_scalarRenderField, simulation.m_velocityRenderField);
	Renderer.Init();

	// Initialize GUI
	GUI gui(&m_DomainConfig, &simulation_configs, &m_renderConfig, &playback, &frame_profiler, Renderer.GetTexID(), Window.GetNativeWindow());
	gui.Initialize();



	while (!Window.ShouldClose())
	{
		Window.Update(frame_profiler);

		Renderer.UpdateFields(simulation.m_scalarRenderField, simulation.m_velocityRenderField);

		// Draw Gui
		gui.BeginFrame();

		gui.Draw();

		Renderer.BeginFrame();

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

				simulation.Update();

				simulation.RunSimulation();
				Renderer.Update(simulation.m_pixelValueMin, simulation.m_pixelValueMax, simulation.m_velocityMin, simulation.m_velocityMax);

				break;
			}

			case Playback::Reset :
			{
				playback = Playback::Stop;
				simulation.Init();
				Renderer.Reset();
				break;
			}

			case Playback::Stop:
			{
				simulation.Reset();
				break;
			}

		}



		Renderer.DrawFrame();

		gui.EndFrame();

		Window.SwapBuffers();
		Window.PollEvents();

	}

	return 0;
}