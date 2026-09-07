#pragma once

#include "data_type.h"
#include "field_concepts.h"
#include "field.h"





namespace differential_operators {

    template<ArithmeticField T>
    constexpr auto Du (const T& field, float InCellWidth, int x, int y)
    {
        return (field.GetClamped(x + 1, y) - field.GetClamped(x - 1, y)) / (2.0f *InCellWidth);
    }

    template<ArithmeticField T>
    constexpr auto AbsDu (const T& field, float InCellWidth, int x, int y)
    {
        return (std::abs(field.GetClamped(x + 1, y)) - std::abs(field.GetClamped(x - 1, y))) / (2.0f * InCellWidth);
	}

	template<ArithmeticField T>
    constexpr auto Dv (const T& field, float InCellHeight, int x, int y)
    {
        return (field.GetClamped(x, y + 1) - field.GetClamped(x, y - 1)) / (2.0f * InCellHeight);
    }

	template<ArithmeticField T>
    constexpr auto AbsDv(const T& field, float InCellHeight, int x, int y)
    {
        return (std::abs(field.GetClamped(x, y + 1)) - std::abs(field.GetClamped(x, y - 1))) / (2.0f * InCellHeight);
    }

    // =========================================
	// CentralDivergence Operator
    // =========================================

	template<VectorField T>
    constexpr float BackwardDivergence(const T& field, Float2 InCellSize, int x, int y)
    {
        float du = (field.GetClamped(x, y).x - field.GetClamped(x - 1, y).x) / InCellSize.x;
        float dv = (field.GetClamped(x, y).y - field.GetClamped(x, y - 1).y) / InCellSize.y;
        return du + dv;
	}

	template<VectorField T>
    constexpr float CentralDivergence(const T& field, Float2 InCellSize, int x, int y)
    {
        float du = (field.GetClamped(x + 1, y).x - field.GetClamped(x - 1, y).x) / (2.0f * InCellSize.x);
        float dv = (field.GetClamped(x, y + 1).y - field.GetClamped(x, y - 1).y) / (2.0f * InCellSize.y);
        return du + dv;
	}

    inline float MACGridDivergence (const MACGridVelocityField2D& InMacVelocityField, Float2 InCellSize, int InCellCenterX, int InCellCenterY)
    {
        float du = (InMacVelocityField.GetU(InCellCenterX + 1, InCellCenterY) - InMacVelocityField.GetU(InCellCenterX, InCellCenterY)) / InCellSize.x;
        float dv = (InMacVelocityField.GetV(InCellCenterX, InCellCenterY + 1) - InMacVelocityField.GetV(InCellCenterX, InCellCenterY)) / InCellSize.y;

        return du + dv;
    }


	// =========================================
	// Laplacian Operator
	// =========================================
	template <ArithmeticField T>
    constexpr auto Laplacian (const T& field, Float2 InCellSize, int x, int y)
    {
        return (field.GetClamped(x + 1, y) - 2.0f * field.GetClamped(x, y) + field.GetClamped(x - 1, y)) / (InCellSize.x * InCellSize.x)
             + (field.GetClamped(x, y + 1) - 2.0f * field.GetClamped(x, y) + field.GetClamped(x, y - 1)) / (InCellSize.y * InCellSize.y);
    }

    //========================================
	// Step Diffusion Operator
	//========================================
	template <ArithmeticField T>
    constexpr auto Diffusion (const T& field, const float diffusion_decay_rate, const float diffusion_coefficient, Float2 InCellSize, int x, int y)
    {
        auto Value = field.Get(x, y);
        auto LaplacianValue = Laplacian(field, InCellSize, x, y);

        auto NewValue = Value + diffusion_coefficient * LaplacianValue * diffusion_decay_rate;

		return NewValue;
	}
}