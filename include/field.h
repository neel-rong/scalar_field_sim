#pragma once

#include "data_type.h"
#include "field_concepts.h"
#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "interpolation_operators.h"


template<typename T>
class Field2D
{

public:

	using ValueType = T;


	// =====================================
	// Constructor and Destructor
	// =====================================
	Field2D() : m_width(0), m_height(0) {} // default constructor for uninitialized field


	// =====================================
	// Properties
	// =====================================

	static constexpr int GhostCellPadding = 1; // Denotes the ghost cells for boundary condition implementation


	// =====================================
	// Functions
	// =====================================

	constexpr ValueType Get(int x, int y) const
	{
		return m_data[Index(x, y)];
	}

	constexpr ValueType GetClamped(int x, int y) const
	{
		x = std::max(0, std::min(x, m_width - 1));
		y = std::max(0, std::min(y, m_height - 1));
		return m_data[Index(x, y)];
	}

	constexpr ValueType GetWrapped(int x, int y) const
	{
		x = (x % m_width + m_width) % m_width;
		y = (y % m_height + m_height) % m_height;
		return m_data[Index(x, y)];
	}

	constexpr ValueType GetZeroBoundary(int x, int y) const
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		{
			return ValueType{};
		}
		return m_data[Index(x, y)];
	}

	int GetInteriorMaxX() const { return m_width - 1 - GhostCellPadding; }
	int GetInteriorMaxY() const { return m_height - 1 - GhostCellPadding; }
	int GetInteriorMinX() const { return GhostCellPadding; }
	int GetInteriorMinY() const { return GhostCellPadding; }


	//Set Value at 2D coordinates
	void Set(int x, int y, ValueType InValue)
	{
		m_data[Index(x, y)] = InValue;
	}

	void SetInterior(int x, int y, ValueType InValue)
	{
		assert(x >= GhostCellPadding && x <= m_width - 1 - GhostCellPadding);
		assert(y >= GhostCellPadding && y <= m_height - 1 - GhostCellPadding);

		Set(x, y, InValue);
	}

	void UpdateGhostCellsNeumann()
	{
		for (int y = GhostCellPadding; y < m_height - GhostCellPadding; ++y)
		{
			m_data[Index(0, y)] = m_data[Index(GhostCellPadding, y)];
			m_data[Index(m_width - 1, y)] = m_data[Index(m_width - 1 - GhostCellPadding, y)];
		}

		for (int x = GhostCellPadding; x < m_width - GhostCellPadding; ++x)
		{
			m_data[Index(x, 0)] = m_data[Index(x, GhostCellPadding)];
			m_data[Index(x, m_height - 1)] = m_data[Index(x, m_height - 1 - GhostCellPadding)];
		}

		m_data[Index(0, 0)] = m_data[Index(1, 1)];
		m_data[Index(0, m_height - 1)] = m_data[Index(1, m_height - GhostCellPadding - 1)];
		m_data[Index(m_width - 1, 0)] = m_data[Index(m_width - GhostCellPadding - 1, 1)];
		m_data[Index(m_width - 1, m_height - 1)] = m_data[Index(m_width - GhostCellPadding - 1, m_height - GhostCellPadding - 1)];
	}


	//Dimensions
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	int GetInteriorWidth() const { return m_width - (GhostCellPadding * 2); }
	int GetInteriorHeight() const { return m_height - (GhostCellPadding * 2); }

	Float2 GetSamplingOffset() const { return m_samplingOffset; }

	//Utility
	void Fill(ValueType InValue)
	{
		std::fill(m_data.begin(), m_data.end(), InValue);
	}

	constexpr auto Sample(Float2 InLocalPosition) const
	{
		auto GetClamped = [this](int x, int y)
			{
				return this->GetClamped(x, y);
			};

		return interpolation::Bilinear(InLocalPosition - m_samplingOffset, GetClamped);
	}

	constexpr const std::vector<ValueType>& GetData() const
	{
		return m_data;
	}

	void Init(int InWidth, int InHeight, ValueType InInitialValue, Float2 InSamplingOffset = Float2(0.5f, 0.5f))
	{
		m_width = InWidth + (GhostCellPadding * 2);
		m_height = InHeight + (GhostCellPadding * 2);

		m_samplingOffset = InSamplingOffset;

		m_data.assign(m_width * m_height, InInitialValue);
	}

	void Reset(ValueType InValue)
	{
		m_data.assign(m_width * m_height, InValue);
	}

	void Clear()
	{
		m_data.clear();
	}

private:

	int m_width;
	int m_height;

	std::vector<ValueType> m_data;

	Float2 m_samplingOffset = Float2(0.5f, 0.5f);


	int Index(int x, int y) const
	{
		assert(x >= 0 && x < m_width);
		assert(y >= 0 && y < m_height);

		return y * m_width + x;
	}

};





// ===================================================================
// MAC GRID Field
// ===================================================================

class MACGridVelocityField2D
{
public:

	// =========================================================================================================================================================
	// Constructor
	// =========================================================================================================================================================

	// Default Constructor
	MACGridVelocityField2D() {}

	// =========================================================================================================================================================
	// Properties
	// =========================================================================================================================================================
	static constexpr int GhostCellPadding = 1;


	// =========================================================================================================================================================
	// Core Functions
	// =========================================================================================================================================================

	void Init(int InWidth, int InHeight, float InInitalValue)
	{
		m_width = InWidth;
		m_height = InHeight;

		VelocityU.Init(m_width + 1, m_height, InInitalValue, Float2(0.0f, 0.5f));
		VelocityV.Init(m_width, m_height + 1, InInitalValue, Float2(0.5f, 0.0f));
	}

	void Reset(float InValue)
	{
		VelocityU.Reset(InValue);
		VelocityV.Reset(InValue);
	}

	// =========================================================================================================================================================
	// Get Functions
	// =========================================================================================================================================================



	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	Field2D<float>& UField()
	{
		return VelocityU;
	}

	const Field2D<float>& UField() const
	{
		return VelocityU;
	}

	Field2D<float>& VField()
	{
		return VelocityV;
	}

	const Field2D<float>& VField() const
	{
		return VelocityV;
	}

	// Returns the value of the x and y index of the array grid
	// Repersents the Physical MAC Grid Coordinates excluding the ghost cells
	// U: x = [0, m_width];	y = [0, m_height)
	// V: x = [0, m_width);	y = [0, m_height]

	float GetU(int x, int y) const
	{
		assert(x >= 0 && x <= m_width);
		assert(y >= 0 && y < m_height);

		return UField().Get(x + GhostCellPadding, y + GhostCellPadding);
	}

	float GetV(int x, int y) const
	{
		assert(x >= 0 && x < m_width);
		assert(y >= 0 && y <= m_height);

		return VField().Get(x + GhostCellPadding, y + GhostCellPadding);
	}

	// Returns velocity interpolated from MAC face values to the cell center.
	Float2 GetVelocityAtCellCenter(int x, int y) const
	{
		return Float2((GetU(x, y) + GetU(x + 1, y)) * 0.5f,
					  (GetV(x, y) + GetV(x, y + 1)) * 0.5f
		);
	}

	Float2 GetSample(Float2 InLocalPosition) const
	{
		return Float2(
			VelocityU.Sample(InLocalPosition),
			VelocityV.Sample(InLocalPosition)
		);
	}

	// The MACGrid hides the internal ghost-cell padding of the constituent Field2D.
	// Consequently, converting a MACGrid cell coordinate to Field2D grid space
	// requires an offset of GhostCellPadding.
	// The additional 0.5f places the position at the cell center.
	Float2 GetCellCenterPosition(int x, int y)
	{
		return Float2( x + GhostCellPadding + 0.5f,
					   y + GhostCellPadding + 0.5f
		);
	}



	// =========================================================================================================================================================
	// Set Functions
	// =========================================================================================================================================================

	void Fill(float InValue)
	{
		VelocityU.Fill(InValue);
		VelocityV.Fill(InValue);
	}


	// =========================================================================================================================================================
	// Boundary Conditions
	// =========================================================================================================================================================


	// Sets the normal component of the velocity at the boundary to zero to create a no-flux boundary condition for the advection simulation
	void SetBoundaryNormalComponentZero()
	{
		int padding = Field2D<float>::GhostCellPadding;

		for (int y = padding; y < VelocityU.GetHeight() - padding; ++y)
		{
			VelocityU.Set(padding, y, 0.0f);
			VelocityU.Set(VelocityU.GetWidth() - 1 - padding, y, 0.0f);
		}

		for (int x = padding; x < VelocityV.GetWidth() - padding; ++x)
		{
			VelocityV.Set(x, padding, 0.0f);
			VelocityV.Set(x, VelocityV.GetHeight() - 1 - padding, 0.0f);
		}
	}


	// Sets the Ghost Cells to Neumann Boundary Condition
	void UpdateGhostCellsNeumann()
	{
		VelocityU.UpdateGhostCellsNeumann();
		VelocityV.UpdateGhostCellsNeumann();
	}


private:

	Field2D<float> VelocityU;
	Field2D<float> VelocityV;

	int m_width = 0;
	int m_height = 0;
};



// ===================================================================
// Fields Struct
// ===================================================================

struct Fields
{
	// Scalar Fields
	Field2D<float> DensityField;
	Field2D<float> DivergenceField;
	Field2D<float> PressureField;
	Field2D<float> ResidualField;

	// MACGrid Field
	MACGridVelocityField2D VelocityField;

	// Scrath Fields
	Field2D<float> ScalarFieldScratch;
	MACGridVelocityField2D VelocityFieldScratch;


	void InitializeFields(Int2 GridResolution, float InValue)
	{
		DensityField.Init(GridResolution.x, GridResolution.y, InValue);
		DivergenceField.Init(GridResolution.x, GridResolution.y, InValue);
		PressureField.Init(GridResolution.x, GridResolution.y, InValue);
		ResidualField.Init(GridResolution.x, GridResolution.y, InValue);

		VelocityField.Init(GridResolution.x, GridResolution.y, InValue);

		ScalarFieldScratch.Init(GridResolution.x, GridResolution.y, InValue);
		VelocityFieldScratch.Init(GridResolution.x, GridResolution.y, InValue);
	}

	void ResetFields(float InValue)
	{
		DensityField.Reset(InValue);
		DivergenceField.Reset(InValue);
		PressureField.Reset(InValue);
		ResidualField.Reset(InValue);

		VelocityField.Reset(InValue);

		ScalarFieldScratch.Reset(InValue);
		VelocityFieldScratch.Reset(InValue);
	}
};


struct DebugFields
{
	// Scalar Fields
	Field2D<float> DivergenceFieldBeforeProjection;
	Field2D<float> DivergenceFieldAfterProjection;


	void InitializeFields(Int2 GridResolution, float InValue)
	{
		DivergenceFieldBeforeProjection.Init(GridResolution.x, GridResolution.y, InValue);
		DivergenceFieldAfterProjection.Init(GridResolution.x, GridResolution.y, InValue);
	}

	void ResetFields(float InValue)
	{
		DivergenceFieldBeforeProjection.Reset(InValue);
		DivergenceFieldAfterProjection.Reset(InValue);
	}
};