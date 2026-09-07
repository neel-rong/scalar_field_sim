#pragma once
#include "field.h"
#include "differential_operators.h"
#include "sim_solvers.h"
#include "misc_operators.h"
#include "debug_helpers.h"
#include "sim_properties.h"


namespace sim_ops
{
	// ============================================================================================================================================
	// Source =====================================================================================================================================

	void ApplySource(Field2D<float>& InField, float InValue, bool bUpdateGhostCellsNeumann);

	void ApplySource(Field2D<float>& InField, float InMinValue, float InMaxValue, bool bUpdateGhostCellsNeumann);


	void InjectVelocityAtCell(Field2D<float>& InField, Float2 InCellSize, float InRadius, Double2 InPosition, Double2 InVelocity);
	void InjectVelocityAtCell(MACGridVelocityField2D& InField, Float2 InCellSize, float InRadius, Double2 InPosition, Double2 InVelocity);

	void InjectionVelocityFromMouseState(MACGridVelocityField2D& InVelocityField, const MouseState* InMouseState, const SimulationConfig* InSimulationConfig, const DomainConfig* InDomainConfig);


	// Advection ==================================================================================================================================

	void Advection(const Field2D<float>& InField, Field2D<float>& OutField, const MACGridVelocityField2D& InVelocityField, const Float2 InCellSize, float dt);


	void Advection(const MACGridVelocityField2D& InVelocityField, MACGridVelocityField2D& OutVelocityField, const Float2 InCellSize, float dt);


	// ============================================================================================================================================
	// Divergence =================================================================================================================================

	void Divergence(const MACGridVelocityField2D& InField, Field2D<float>& OutField, const Float2 InCellSize);


	// =============================================================================================================================================
	// Laplacian ===================================================================================================================================

	template<ArithmeticField T>
	void Laplacian(const T& InField, T& OutField)
	{
		for (int y = Field2D<float>::GhostCellPadding; y < InField.GetHeight() - Field2D<float>::GhostCellPadding; y++)
		{
			for (int x = Field2D<float>::GhostCellPadding; x < InField.GetWidth() - Field2D<float>::GhostCellPadding; ++x)
			{
				auto value = differential_operators::Laplacian(InField, x, y);
				OutField.Set(x, y, value);
			}
		}
	}


	// ==============================================================================================================================================
	// Diffusion ====================================================================================================================================

	template<ArithmeticField T>
	void Diffusion(const T& InField, T& OutField, const float DiffusionDecayRate, const float DiffusionCoefficient)
	{
		for (int y = Field2D<float>::GhostCellPadding; y < InField.GetHeight() - Field2D<float>::GhostCellPadding; y++)
		{
			for (int x = Field2D<float>::GhostCellPadding; x < InField.GetWidth() - Field2D<float>::GhostCellPadding; ++x)
			{
				auto value = differential_operators::Diffusion(InField, DiffusionDecayRate, DiffusionCoefficient, x, y);
				OutField.Set(x, y, value);
			}
		}
	}


	// ==============================================================================================================================================
	// Pressure Solve ===============================================================================================================================

	void Jacobi(Field2D<float>& InPressureField, Field2D<float>& TemporaryField, const Field2D<float>& DivergenceField, const Float2 InCellSize, int InIterations);


	void GaussSeidel(Field2D<float>& InPressureField, const Field2D<float>& InDivergenceField, float InOmega, const Float2 InCellSize, int InIterations);


	void RedBlackGaussSeidel(Field2D<float>& InPressureField, const Field2D<float>& InDivergenceField, float InOmega, const Float2 InCellSize, int InIterations);


	// ============================================================================================================================================
	// Residual ===================================================================================================================================
	void ComputePressureResidual(const Field2D<float>& InPressureField, const Field2D<float>& InDivergenceField, Field2D<float>& OutField, const Float2 InCellSize);


	// ============================================================================================================================================
	//  Velocity Projection =======================================================================================================================

	void SimulateVelocityProjection(const Field2D<float>& InPressureField, const MACGridVelocityField2D& InVelocityField, MACGridVelocityField2D& OutVelocityField, const Float2 InCellSize);


	// ============================================================================================================================================
	// Stats ======================================================================================================================================
	template<typename T>
		requires std::is_arithmetic_v<T>
	void ComputeStats(const Field2D<T>& InField, FieldStats<T>& InStats)
	{
		for (int y = 0; y < InField.GetHeight(); ++y)
		{
			for (int x = 0; x < InField.GetWidth(); ++x)
			{
				T value = InField.Get(x, y);

				InStats.Update(value);
			}
		}
	}

	void ComputeStats(const MACGridVelocityField2D& InField, MACGridVelocityStats& InStats);

	template <ArithmeticField T, typename SampleFunction>
	void ComputeStats(const T& InField, FieldStats<T>& InStats, SampleFunction Function)
	{
		for (int y = 0; y < InField.GetHeight(); ++y)
		{
			for (int x = 0; x < InField.GetWidth(); ++x)
			{
				T value = Function(x, y);

				InStats.Update(value);
			}
		}
	}


	// ============================================================================================================================================
	// ============================================================================================================================================


	bool ExecuteSimStep(Fields& InFields, DomainConfig* InDomainConfig, SimulationConfig* InSimulationConfig, DebugFields* InDebugFields = nullptr, SimStepStats* InSimStepStats = nullptr);
}