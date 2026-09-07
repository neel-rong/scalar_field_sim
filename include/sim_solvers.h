#pragma once

#include "field.h"
#include "interpolation_operators.h"


namespace sim_solvers
{
	// =========================================
	// Advection Operator
	// =========================================

	inline float AdvectByNearestNeighbour(const Field2D<float>& curr_field, Field2D<float>& next_field, Float2 velocity, Float2 InCellSize, int x, int y, float dt)
	{
		float prev_x = x - ((velocity.x * dt) / InCellSize.x);
		float prev_y = y - ((velocity.y * dt) / InCellSize.y);

		Int2 SampleCoor = interpolation::NearestNeighbour(prev_x, prev_y);

		float value = curr_field.GetClamped(SampleCoor.x, SampleCoor.y);

		return value;
	}

	inline float AdvectByBilinear(const Field2D<float>& InField, const MACGridVelocityField2D& InVelocityField, const Float2 InPosition, const Float2 InCellSize, const float dt)
	{
		Float2 velocityAtCenter = InVelocityField.GetSample(InPosition);

		float previousPositionX = InPosition.x - ((velocityAtCenter.x * dt) / InCellSize.x);
		float previousPositionY = InPosition.y - ((velocityAtCenter.y * dt) / InCellSize.y);

		return InField.Sample(Float2(previousPositionX, previousPositionY));
	}

	// Note: We get the equation from pressure (p) as follows:
	//		u' = u - grad(p)
	// =>	divergence(u') = divergence(u - (grad(p))
	// =>	divergence(u') = divergence(u) - laplacian(p)
	//
	// Enforce Incompressibility: divergence(u') = 0
	//
	// Therefore,
	//		divergence(u) - laplacian(p) = 0
	// =>	laplacian(p) = divergence(u)
	// =>	(pE - 2pC +pW)/dx^2 + (pN - 2pC + pS)/dy^2 = D
	// =>	pC = ((pE + pW)/dx^2 + (pN + pS)/dy^2 - D) / ((2/dx^2) + (2/dy^2))
	//

	inline float PressureSolver(const Field2D<float>& InPressureField, const Field2D<float>& InDivergenceField, Float2 InCellSize, int x, int y)
	{
		float pE = InPressureField.Get(x + 1, y);
		float pW = InPressureField.Get(x - 1, y);
		float pN = InPressureField.Get(x, y + 1);
		float pS = InPressureField.Get(x, y - 1);

		return ((pE + pW) / (InCellSize.x * InCellSize.x) + (pN + pS) / (InCellSize.y * InCellSize.y) - InDivergenceField.Get(x, y)) /
			((2.0f / (InCellSize.x * InCellSize.x)) + (2.0f / (InCellSize.y * InCellSize.y)));
	}

	inline float PressureSolverSOR(const Field2D<float>& InPressureField, const Field2D<float>& InDivergenceField, float omega, Float2 InCellSize, int x, int y)
	{
		float pNew = PressureSolver(InPressureField, InDivergenceField, InCellSize, x, y);
		float pOld = InPressureField.Get(x, y);

		return pOld + omega * (pNew - pOld);
	}
}