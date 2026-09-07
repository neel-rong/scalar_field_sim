#include "simulation.h"



// =====================================================================================
// Core Functions
// =====================================================================================

void Simulation::Init()
{
	m_fields.InitializeFields(m_domainConfig->GridResolution, 0.0f);

	m_pixelValueMin = 0.0f;
	m_pixelValueMax = 0.0f;

	m_dyeEmitter.Init(&m_simulationConfig->DensityEmitterProperties);
	m_velocityUEmitter.Init(&m_simulationConfig->VelocityUEmitterProperties);
	m_velocityVEmitter.Init(&m_simulationConfig->VelocityVEmitterProperties);
}

void Simulation::Reset()
{
	m_fields.ResetFields(0.0f);

	m_pixelValueMin = 0.0f;
	m_pixelValueMax = 0.0f;

	m_dyeEmitter.Reset();
	m_velocityUEmitter.Reset();
	m_velocityVEmitter.Reset();
}

void Simulation::Update()
{
	m_dyeEmitter.Update(m_simulationConfig->TimeStep);
	m_velocityUEmitter.Update(m_simulationConfig->TimeStep);
	m_velocityVEmitter.Update(m_simulationConfig->TimeStep);
}

// =====================================================================================
// Simulation Functions
// =====================================================================================


int Simulation::RunSimulation()
{
	// Apply Sources

	switch (m_simulationConfig->SimulationMode)
	{
	case SimulationConfig::SimulationModeEnum::Test:
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
	case SimulationConfig::SimulationModeEnum::Simulation:
	{
		m_dyeEmitter.Inject(m_fields.DensityField, m_domainConfig->GetCellSize(), m_simulationConfig->TimeStep);
		m_velocityUEmitter.Inject(m_fields.VelocityField.UField(), m_domainConfig->GetCellSize(), m_simulationConfig->TimeStep);
		m_velocityVEmitter.Inject(m_fields.VelocityField.VField(), m_domainConfig->GetCellSize(), m_simulationConfig->TimeStep);
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
	if (!sim_ops::ExecuteSimStep(m_fields, m_domainConfig, m_simulationConfig))
	{ return -1; }

	// Set Render Fields
	if (m_renderConfig->ScalarMode == RenderConfig::ScalarVisualizationMode::DensityField) m_scalarRenderField = &m_fields.DensityField;
	if (m_renderConfig->ScalarMode == RenderConfig::ScalarVisualizationMode::PressureField) m_scalarRenderField = &m_fields.PressureField;
	if (m_renderConfig->ScalarMode == RenderConfig::ScalarVisualizationMode::VelocityDivergenceField) m_scalarRenderField = &m_fields.DivergenceField;

	m_velocityRenderField = &m_fields.VelocityField;

	// Update Pixel Min Max
	UpdatePixelMinMaxValue(m_scalarRenderField);
	UpdateVelocityMinMaxValue(m_velocityRenderField);

	return 0;

}


// =====================================================================================
// Helper Functions
// =====================================================================================

// Update visualization ranges
void Simulation::UpdatePixelMinMaxValue(const Field2D<float>* InField)
{
	m_pixelValueMax = std::numeric_limits<float>::lowest();
	m_pixelValueMin = std::numeric_limits<float>::max();

	grid_algorithms::ConstFieldGridIterator(InField, [&](int x, int y)
		{
			float value = InField->Get(x, y);

			m_pixelValueMin = std::min(m_pixelValueMin, value);
			m_pixelValueMax = std::max(m_pixelValueMax, value);
		});
}


void Simulation::UpdateVelocityMinMaxValue(const MACGridVelocityField2D* InField)
{
	m_velocityMax.x = std::numeric_limits<float>::lowest();
	m_velocityMax.y = std::numeric_limits<float>::lowest();
	m_velocityMin.x = std::numeric_limits<float>::max();
	m_velocityMin.y = std::numeric_limits<float>::max();

	const Field2D<float>& U = InField->UField();
	const Field2D<float>& V = InField->VField();

	grid_algorithms::ConstFieldGridIterator(U, [&](int x, int y)
		{
			float value = U.Get(x, y);

			m_velocityMin = Float2(std::min(m_velocityMin.x, value), m_velocityMin.y);
			m_velocityMax = Float2(std::max(m_velocityMax.x, value), m_velocityMax.y);
		});

	grid_algorithms::ConstFieldGridIterator(V, [&](int x, int y)
		{
			float value = V.Get(x, y);

			m_velocityMin = Float2(m_velocityMin.x, std::min(m_velocityMin.y, value));
			m_velocityMax = Float2(m_velocityMax.x, std::max(m_velocityMax.y, value));
		});
}