#include "simulation.h"



// =====================================================================================
// Core Functions
// =====================================================================================

void Simulation::Init()
{
	m_fields.InitializeFields(m_domainConfig->GridResolution, 0.0f);

	m_pixelValueMin = 0.0f;
	m_pixelValueMax = 0.0f;

	m_simulationTimeStep = m_simulationConfig->TargetTimeStep;

	DensityStats.Reset();
	DivergenceStats.Reset();
	PressureStats.Reset();

	m_Emitter.Init(&m_simulationConfig->EmitterProperty);

	Update();
}

void Simulation::Reset()
{
	m_fields.ResetFields(0.0f);

	m_pixelValueMin = 0.0f;
	m_pixelValueMax = 0.0f;

	m_simulationTimeStep = m_simulationConfig->TargetTimeStep;

	DensityStats.Reset();
	DivergenceStats.Reset();
	PressureStats.Reset();

	m_Emitter.Reset();
}

void Simulation::Update()
{
	// Set Render Fields
	if (m_renderConfig->ScalarMode == RenderConfig::ScalarVisualizationMode::DensityField) m_renderFieldScalar = &m_fields.DensityField;
	if (m_renderConfig->ScalarMode == RenderConfig::ScalarVisualizationMode::PressureField) m_renderFieldScalar = &m_fields.PressureField;
	if (m_renderConfig->ScalarMode == RenderConfig::ScalarVisualizationMode::VelocityDivergenceField) m_renderFieldScalar = &m_fields.DivergenceField;
}

// =====================================================================================
// Simulation Functions
// =====================================================================================


int Simulation::RunSimulation()
{
	float timeStep;

	if (m_simulationConfig->bUseCFLTimeStep)
	{
		m_simulationTimeStep = sim_ops::ComputeCFLTimeStep(m_fields.VelocityField, m_domainConfig->GetCellSize(), m_simulationConfig->CFLNumber, m_simulationConfig->TargetTimeStep);
		timeStep = m_simulationTimeStep;
	}
	else
	{
		timeStep = m_simulationConfig->TargetTimeStep;
	}

	for (int i = 0; i < 1; ++i)
	{
		// Apply Sources
		switch (m_simulationConfig->SimulationMode)
		{
		case SimulationConfig::SimulationModeEnum::Test:			// Soure for Test Mode
		{
			switch (m_simulationConfig->TestMode)
			{
			case SimulationConfig::TestModeEnum::DivergenceTest:
			{
				m_fields.VelocityField.Fill(1.0f);
				break;
			}
			case SimulationConfig::TestModeEnum::PressureTest:
			{
				sim_ops::ApplySource(m_fields.VelocityField.VField(), 0.0f, 1.0f, false);
				m_fields.VelocityField.SetBoundaryNormalComponentZero();
				m_fields.VelocityField.UpdateGhostCellsNeumann();
				break;
			}
			default:
			{
				std::cout << "Invalid Test Mode";
				return -1;
			}
			}
			break;
		}
		case SimulationConfig::SimulationModeEnum::Simulation:		// Source for Simulatio Mode
		{
			m_Emitter.Inject(m_fields, m_domainConfig->GetCellSize(), timeStep);

			sim_ops::InjectionVelocityFromMouseState(m_fields.VelocityField, m_mouseState, m_simulationConfig, m_domainConfig);
			break;
		}

		default:
		{
			std::cout << "Invalid Simulation Mode";
			return -1;
		}
		}

		// Execute sim step
		if (!sim_ops::ExecuteSimStep(m_fields, m_domainConfig, m_simulationConfig, timeStep))
		{
			return -1;
		}
	}

	UpdatePixelMinMax();

	return 0;
}


// Helper Functions
void Simulation::UpdatePixelMinMax()
{
	FieldStats<float>* stats = nullptr;

	if (m_renderConfig->ScalarMode == RenderConfig::ScalarVisualizationMode::DensityField) stats = &DensityStats;
	if (m_renderConfig->ScalarMode == RenderConfig::ScalarVisualizationMode::PressureField) stats = &DivergenceStats;
	if (m_renderConfig->ScalarMode == RenderConfig::ScalarVisualizationMode::VelocityDivergenceField) stats = &PressureStats;


	if (stats !=nullptr)
	{
		sim_ops::ComputeStats(m_fields.DensityField, *stats);

		m_renderFieldVelocity = &m_fields.VelocityField;

		// Update Pixel Min Max
		m_pixelValueMin = stats->GetStatsMin();
		m_pixelValueMax = stats->GetStatsMax();
	}
	else
	{
		std::cout << "Invalid Field Stats";
	}
}
