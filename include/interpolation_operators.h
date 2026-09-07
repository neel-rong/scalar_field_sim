#pragma once

#include "data_type.h"
#include "field_concepts.h"


namespace interpolation {

	// =========================================
	// Nearest Neighbour Interpolation
	// =========================================
	inline Int2 NearestNeighbour(float x, float y)
	{
		int x0 = static_cast<int>(std::round(x));
		int y0 = static_cast<int>(std::round(y));

		return Int2(x0, y0);
	}

	// =========================================
	// Bilinear Interpolation
	// =========================================
	template<typename SampleFunction>
	constexpr auto Bilinear(Float2 InLocalPosition, SampleFunction InGetFunction)
	{
		float x0 = std::floor(InLocalPosition.x);
		float y0 = std::floor(InLocalPosition.y);

		float x1 = x0 + 1;
		float y1 = y0 + 1;

		float tx = InLocalPosition.x - x0;
		float ty = InLocalPosition.y - y0;

		auto v00 = InGetFunction(static_cast<int>(x0), static_cast<int>(y0));
		auto v10 = InGetFunction(static_cast<int>(x1), static_cast<int>(y0));
		auto v01 = InGetFunction(static_cast<int>(x0), static_cast<int>(y1));
		auto v11 = InGetFunction(static_cast<int>(x1), static_cast<int>(y1));

		auto Top = v00 * (1 - tx) + v10 * tx;
		auto Bottom = v01 * (1 - tx) + v11 * tx;

		auto value = Top * (1 - ty) + Bottom * ty;

		return value;
	}

}