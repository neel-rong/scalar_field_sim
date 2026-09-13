#include "simulation_operators.h"
#include "simulation_operators.h"
#include <limits>
#include "simulation_operators.h"




// ============================================================================================================================================
// Source =====================================================================================================================================

void sim_ops::ApplySource(Field2D<float>& InField, float InValue, bool bUpdateGhostCellsNeumann)
{
	InField.Fill(InValue);

	if (bUpdateGhostCellsNeumann)
	{
		InField.UpdateGhostCellsNeumann();
	}
}

void sim_ops::ApplySource(Field2D<float>& InField, float InMinValue, float InMaxValue, bool bUpdateGhostCellsNeumann)
{
	float delta = (InMinValue - InMaxValue) / InField.GetHeight();

	for (int y = Field2D<float>::GhostCellPadding; y < InField.GetHeight() - Field2D<float>::GhostCellPadding; y++)
	{
		float value = InMinValue - (y * delta);

		for (int x = Field2D<float>::GhostCellPadding; x < InField.GetWidth() - Field2D<float>::GhostCellPadding; ++x)
		{
			InField.Set(x, y, value);
		}
	}

	if (bUpdateGhostCellsNeumann)
	{
		InField.UpdateGhostCellsNeumann();
	}
}


void sim_ops::InjectVelocityAtCell(Field2D<float>& InField, Float2 InCellSize, float InRadius, Double2 InPosition, Double2 InVelocity)
{
	float vx = static_cast<float>(InVelocity.x);
	float vy = static_cast<float>(InVelocity.y);

	Float2 pos = Float2(static_cast<float>(InPosition.x), static_cast<float>(InPosition.y));

	grid_algorithms::ForEachCellInRadius(InCellSize, InRadius, pos, [&](int x, int y)
		{
			if (x >= 0 && x < InField.GetWidth() && y >= 0 && y < InField.GetHeight())
			{
				InField.Set(x, y, InField.Get(x, y) + (vx * 0.5f) + (vy * 0.5f));
			}
		});
}


void sim_ops::InjectVelocityAtCell(MACGridVelocityField2D& InField, Float2 InCellSize, float InRadius, Double2 InPosition, Double2 InVelocity)
{
	Field2D<float>& U = InField.UField();
	Field2D<float>& V = InField.VField();

	float vx = static_cast<float>(InVelocity.x);
	float vy = static_cast<float>(InVelocity.y);

	Float2 pos = Float2(static_cast<float>(InPosition.x), static_cast<float>(InPosition.y));

	grid_algorithms::ForEachCellInRadius(InCellSize, InRadius, pos, [&](int x, int y)
		{
			if (x >= 0 && x < U.GetWidth() && y >= 0 && y < U.GetHeight())
			{
				U.Set(x, y, U.Get(x, y) + (vx * 0.5f));
			}

			if (x + 1 >= 0 && x + 1 < U.GetWidth() && y >= 0 && y < U.GetHeight())
			{
				U.Set(x + 1, y, U.Get(x + 1, y) + (vx * 0.5f));
			}

			if (x >= 0 && x < V.GetWidth() && y >= 0 && y < V.GetHeight())
			{
				V.Set(x, y, V.Get(x, y) + (vy * 0.5f));
			}
			if (x >= 0 && x < V.GetWidth() && y + 1 >= 0 && y + 1 < V.GetHeight())
			{
				V.Set(x, y + 1, V.Get(x, y + 1) + (vy * 0.5f));
			}
		});
}

void sim_ops::InjectionVelocityFromMouseState(MACGridVelocityField2D& InVelocityField, const MouseState* InMouseState, const SimulationConfig* InSimulationConfig, const DomainConfig* InDomainConfig)
{
	if (!InMouseState->bLeftButtonDown)
		return;

	if (!InMouseState->IsMouseInsideGrid())
		return;

	Double2 start = InMouseState->PreviousGridPosition;

	Double2 deltaMousePos = InMouseState->DeltaGrid;

	Double2 mouseVelocity = InMouseState->MouseVelocity;

	Double2 distance = Double2(std::abs(deltaMousePos.x), (std::abs(deltaMousePos.y)));

	int steps = std::max(1, static_cast<int>(std::max(distance.x, distance.y)));

	int sampleCount = steps + 1;

	mouseVelocity = (mouseVelocity / static_cast<double>(sampleCount)) * InSimulationConfig->VelocityMouseInjectionScale;

	for (int i = 0; i <= steps; ++i)
	{
		double t = static_cast<double>(i) / static_cast<double>(steps);

		double px = start.x + deltaMousePos.x * t;
		double py = start.y + deltaMousePos.y * t;

		sim_ops::InjectVelocityAtCell(InVelocityField, InDomainConfig->GetCellSize(), InSimulationConfig->MouseInjectionRadius, Double2(px, py), mouseVelocity);
	}
}

// ============================================================================================================================================
// Advection ==================================================================================================================================

void sim_ops::Advection(const Field2D<float>& InField, Field2D<float>& OutField, const MACGridVelocityField2D& InVelocityField, const Float2 InCellSize, float dt)
{
	for (int y = Field2D<float>::GhostCellPadding; y < InField.GetHeight() - Field2D<float>::GhostCellPadding; y++)
	{
		for (int x = Field2D<float>::GhostCellPadding; x < InField.GetWidth() - Field2D<float>::GhostCellPadding; ++x)
		{
			Float2 position = Float2(static_cast<float>(x), static_cast<float>(y)) + InField.GetSamplingOffset();
			auto value = sim_solvers::AdvectByBilinear(InField, InVelocityField, position, InCellSize, dt);
			OutField.Set(x, y, value);
		}
	}
}


void sim_ops::Advection(const MACGridVelocityField2D& InVelocityField, MACGridVelocityField2D& OutVelocityField, const Float2 InCellSize, float dt)
{
	Advection(InVelocityField.UField(), OutVelocityField.UField(), InVelocityField, InCellSize, dt);
	Advection(InVelocityField.VField(), OutVelocityField.VField(), InVelocityField, InCellSize, dt);
}


// ============================================================================================================================================
// Divergence =================================================================================================================================

void sim_ops::Divergence(const MACGridVelocityField2D& InField, Field2D<float>& OutField, const Float2 InCellSize)
{
	for (int y = 0; y < InField.GetHeight(); y++)
	{
		for (int x = 0; x < InField.GetWidth(); ++x)
		{
			float value = differential_operators::MACGridDivergence(InField, InCellSize, x, y);
			OutField.Set(x + Field2D<float>::GhostCellPadding, y + Field2D<float>::GhostCellPadding, value);
		}
	}
}


// ==============================================================================================================================================
// Pressure Solve ===============================================================================================================================

void sim_ops::Jacobi(Field2D<float>& InPressureField, Field2D<float>& TemporaryField, const Field2D<float>& DivergenceField, const Float2 InCellSize, int InIterations)
{
	const int heightMin = Field2D<float>::GhostCellPadding;
	const int heightMax = InPressureField.GetHeight() - Field2D<float>::GhostCellPadding;

	const int widthMin = Field2D<float>::GhostCellPadding;
	const int widthMax = InPressureField.GetWidth() - Field2D<float>::GhostCellPadding;

	for (int j = 0; j < InIterations; ++j)
	{
		for (int y = heightMin; y < heightMax; y++)
		{
			for (int x = widthMin; x < widthMax; ++x)
			{
				float value = sim_solvers::PressureSolver(InPressureField, DivergenceField, InCellSize, x, y);

				TemporaryField.Set(x, y, value);
			}
		}

		std::swap(InPressureField, TemporaryField);

		InPressureField.UpdateGhostCellsNeumann();
	}
}


void sim_ops::GaussSeidel(Field2D<float>& InPressureField, const Field2D<float>& InDivergenceField, float InOmega, const Float2 InCellSize, int InIterations)
{
	for (int j = 0; j < InIterations; ++j)
	{
		for (int y = Field2D<float>::GhostCellPadding; y < InPressureField.GetHeight() - Field2D<float>::GhostCellPadding; y++)
		{
			for (int x = Field2D<float>::GhostCellPadding; x < InPressureField.GetWidth() - Field2D<float>::GhostCellPadding; ++x)
			{
				float value = sim_solvers::PressureSolverSOR(InPressureField, InDivergenceField, InOmega, InCellSize, x, y);

				InPressureField.Set(x, y, value);
			}
		}
		InPressureField.UpdateGhostCellsNeumann();
	}
}


void sim_ops::RedBlackGaussSeidel(Field2D<float>& InPressureField, const Field2D<float>& InDivergenceField, float InOmega, const Float2 InCellSize, int InIterations)
{
	Int2 FirstIteratorMinMax = Int2(Field2D<float>::GhostCellPadding, InPressureField.GetHeight() - Field2D<float>::GhostCellPadding);
	Int2 SecondIteratorMinMax = Int2(Field2D<float>::GhostCellPadding, InPressureField.GetWidth() - Field2D<float>::GhostCellPadding);

	for (int j = 0; j < InIterations; ++j)
	{
		for (int color = 0; color < 2; ++color)
		{
			for (int y = FirstIteratorMinMax.x; y < FirstIteratorMinMax.y; ++y)
			{
				int StartX = SecondIteratorMinMax.x + ((SecondIteratorMinMax.x + y + color) & 1);

				for (int x = StartX; x < SecondIteratorMinMax.y; x += 2)
				{
					float value = sim_solvers::PressureSolverSOR(InPressureField, InDivergenceField, InOmega, InCellSize, x, y);

					InPressureField.Set(x, y, value);
				}

				InPressureField.UpdateGhostCellsNeumann();
			}
		}
	}
}


// ============================================================================================================================================
// Residual ===================================================================================================================================
void sim_ops::ComputePressureResidual(const Field2D<float>& InPressureField, const Field2D<float>& InDivergenceField, Field2D<float>& OutField, const Float2 InCellSize)
{
	assert(OutField.GetWidth() == InPressureField.GetWidth() && OutField.GetHeight() == InPressureField.GetHeight());
	assert(OutField.GetWidth() == InDivergenceField.GetWidth() && OutField.GetHeight() == InDivergenceField.GetHeight());

	for (int y = 0; y < OutField.GetHeight(); ++y)
	{
		for (int x = 0; x < OutField.GetWidth(); ++x)
		{
			OutField.Set(x, y, differential_operators::Laplacian(InPressureField, InCellSize, x, y) - InDivergenceField.Get(x, y));
		}
	}
}


// ============================================================================================================================================
//  Velocity Projection =======================================================================================================================

void sim_ops::SimulateVelocityProjection(const Field2D<float>& InPressureField, const MACGridVelocityField2D& InVelocityField, MACGridVelocityField2D& OutVelocityField, const Float2 InCellSize)
{
	const Field2D<float>& U = InVelocityField.UField();
	const Field2D<float>& V = InVelocityField.VField();

	for (int y = U.GhostCellPadding; y < U.GetHeight() - U.GhostCellPadding; ++y)
	{
		for (int x = U.GhostCellPadding; x < U.GetWidth() - U.GhostCellPadding; ++x)
		{
			float pressureGradientX = (InPressureField.Get(x, y) - InPressureField.Get(x - 1, y)) / InCellSize.x;

			OutVelocityField.UField().Set(x, y, U.Get(x, y) - pressureGradientX);
		}
	}

	for (int y = V.GhostCellPadding; y < V.GetHeight() - V.GhostCellPadding; ++y)
	{
		for (int x = V.GhostCellPadding; x < V.GetWidth() - V.GhostCellPadding; ++x)
		{
			float pressureGradientY = (InPressureField.Get(x, y) - InPressureField.Get(x, y - 1)) / InCellSize.y;

			OutVelocityField.VField().Set(x, y, V.Get(x, y) - pressureGradientY);
		}
	}
}

// ============================================================================================================================================
//  Stats =====================================================================================================================================

void sim_ops::ComputeStats(const MACGridVelocityField2D& InField, MACGridVelocityStats& InStats)
{
	const Field2D<float>& U = InField.UField();
	const Field2D<float>& V = InField.VField();

	for (int y = 0; y < U.GetHeight(); ++y)
	{
		for (int x = 0; x < U.GetWidth(); ++x)
		{
			float value = U.Get(x, y);

			InStats.GetStatsUField().Update(value);
		}
	}

	for (int y = 0; y < V.GetHeight(); ++y)
	{
		for (int x = 0; x < V.GetWidth(); ++x)
		{
			float value = V.Get(x, y);

			InStats.GetStatsVField().Update(value);
		}
	}
}


// ============================================================================================================================================
// CFL Time Step ==============================================================================================================================

float sim_ops::ComputeCFLTimeStep(const MACGridVelocityField2D& InVelocityField, const Float2 InCellSize, float InCFLNumber, float InMaxTimeStep)
{
	MACGridVelocityStats VelocityStats;
	ComputeStats(InVelocityField, VelocityStats);

	const float MaxAbsU = std::max(std::abs(VelocityStats.GetStatsUField().GetStatsMin()),
		std::abs(VelocityStats.GetStatsUField().GetStatsMax()));
	const float MaxAbsV = std::max(std::abs(VelocityStats.GetStatsVField().GetStatsMin()),
		std::abs(VelocityStats.GetStatsVField().GetStatsMax()));

	const float dtU = MaxAbsU > 0.0f
		? InCellSize.x / MaxAbsU
		: std::numeric_limits<float>::max();

	const float dtV = MaxAbsV > 0.0f
		? InCellSize.y / MaxAbsV
		: std::numeric_limits<float>::max();

	const float CFLTimeStep = InCFLNumber * std::min(dtU, dtV);

	return std::min(CFLTimeStep, InMaxTimeStep);
}

float sim_ops::ComputeSubStepTime(float InTargetTimeStep, int InSubStepCount)
{
	return InTargetTimeStep / InSubStepCount;
}

int sim_ops::ComputeSubStepCount(float InTargetTimeStep, float InCFLTimeStep)
{
	return std::ceil(InTargetTimeStep / InCFLTimeStep);
}


// ============================================================================================================================================
// Execute Simulation Step ====================================================================================================================
// Executes a simulation step for the given state of the fields
bool sim_ops::ExecuteSimStep(Fields& InField, DomainConfig* InDomainConfig, SimulationConfig* InSimulationConfig, float InTimeStep, DebugFields* InDebugFields, SimStepStats* InSimStepStats)
{
	// Advect Velocity Field
	Advection(InField.VelocityField, InField.VelocityFieldScratch, InDomainConfig->GetCellSize(), InTimeStep);
	std::swap(InField.VelocityField, InField.VelocityFieldScratch);
	InField.VelocityField.SetBoundaryNormalComponentZero();
	InField.VelocityField.UpdateGhostCellsNeumann();

	if (InSimStepStats != nullptr)
	{
		sim_ops::ComputeStats(InField.VelocityField, InSimStepStats->GetStatsVelocityBeforeProjection());
	}


	// Simulate Divergence Field from Velocity Field
	Divergence(InField.VelocityField, InField.DivergenceField, InDomainConfig->GetCellSize());
	InField.DivergenceField.UpdateGhostCellsNeumann();

	if (InDebugFields != nullptr)
	{
		InDebugFields->DivergenceFieldBeforeProjection = InField.DivergenceField;

		if (InSimStepStats != nullptr)
		{
			sim_ops::ComputeStats(InDebugFields->DivergenceFieldBeforeProjection, InSimStepStats->GetStatsDivergenceBeforeProjection());
		}
	}

	// Useful for Tests for measuring convergence without a warm start
	if (InSimulationConfig->bResetPressureField)
	{
		InField.PressureField.Reset(0.0f);
	}

	// Use Divergence Field to simulate Pressure Field and calculate residual
	switch (InSimulationConfig->PressureSolver)
	{
	case SimulationConfig::PressureSolverEnum::Jacobi:
	{
		Jacobi(InField.PressureField, InField.ScalarFieldScratch, InField.DivergenceField, InDomainConfig->GetCellSize(), InSimulationConfig->SolverIterations);
		break;
	}
	case SimulationConfig::PressureSolverEnum::GaussSeidel:
	{
		GaussSeidel(InField.PressureField, InField.DivergenceField, InSimulationConfig->OverRelaxation, InDomainConfig->GetCellSize(), InSimulationConfig->SolverIterations);
		break;
	}
	case SimulationConfig::PressureSolverEnum::RedBlackGaussSeidel:
	{
		RedBlackGaussSeidel(InField.PressureField, InField.DivergenceField, InSimulationConfig->OverRelaxation, InDomainConfig->GetCellSize(), InSimulationConfig->SolverIterations);
		break;
	}
	default:
	{
		std::cout << "\nError: Invalid Pressure Solver Class\n";
		return false;
	}
	}

	if (InSimStepStats != nullptr)
	{
		sim_ops::ComputeStats(InField.PressureField, InSimStepStats->GetStatsPressure());
	}

	ComputePressureResidual(InField.PressureField, InField.DivergenceField, InField.ResidualField, InDomainConfig->GetCellSize());
	InField.ResidualField.UpdateGhostCellsNeumann();

	if (InSimStepStats != nullptr)
	{
		sim_ops::ComputeStats(InField.ResidualField, InSimStepStats->GetStatsResidual());
	}

	SimulateVelocityProjection(InField.PressureField, InField.VelocityField, InField.VelocityFieldScratch, InDomainConfig->GetCellSize());
	std::swap(InField.VelocityField, InField.VelocityFieldScratch);
	InField.VelocityField.SetBoundaryNormalComponentZero();
	InField.VelocityField.UpdateGhostCellsNeumann();

	if (InSimStepStats != nullptr)
	{
		sim_ops::ComputeStats(InField.VelocityField, InSimStepStats->GetStatsVelocityAfterProjection());
	}

	if (InDebugFields != nullptr)
	{
		Divergence(InField.VelocityField, InDebugFields->DivergenceFieldAfterProjection, InDomainConfig->GetCellSize());
		InDebugFields->DivergenceFieldAfterProjection.UpdateGhostCellsNeumann();

		if (InSimStepStats != nullptr)
		{
			sim_ops::ComputeStats(InDebugFields->DivergenceFieldAfterProjection, InSimStepStats->GetStatsDivergenceAfterProjection());
		}
	}

	Advection(InField.DensityField, InField.ScalarFieldScratch, InField.VelocityField, InDomainConfig->GetCellSize(), InTimeStep);
	std::swap(InField.DensityField, InField.ScalarFieldScratch);

	return true;
}
