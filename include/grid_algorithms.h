#pragma once
#include "data_type.h"
#include "field.h"



namespace grid_algorithms
{

	// This function iterates over a grid of given width and height, calling the provided sample function for each cell with its coordinates
	template <typename SampleFunction>
	constexpr void GridIterator(int GridWidth, int GridHeight, SampleFunction Sample)
	{
		for (int x = 1; x < GridWidth - 1; ++x)
		{
			for (int y = 1; y < GridHeight - 1; ++y)
			{
				Sample(x, y);
			}
		}
	}

	template <typename SampleFunction>
	constexpr void GridIterator2(Int2 FirstIteratorMinMax, Int2 SecondIteratorMinMax, SampleFunction Sample)
	{
		for (int y = FirstIteratorMinMax.x; y < FirstIteratorMinMax.y; ++y)
		{
			for (int x = SecondIteratorMinMax.x; x < SecondIteratorMinMax.y; ++x)
			{
				Sample(x, y);
			}
		}
	}


	// This function iterates over the cells of a field, calling the provided sample function for each cell with its coordinates
	template <typename FieldType, typename SampleFunction>
	constexpr void FieldGridIterator (FieldType& field, SampleFunction sample)
	{
		GridIterator(field.GetWidth(), field.GetHeight(), [&](int x, int y)
			{
				sample(x, y);
			}
		);
	}


	// This function iterates over the cells of a const field, calling the provided sample function for each cell with its coordinates
	template <typename FieldType, typename SampleFunction>
	constexpr void ConstFieldGridIterator(const FieldType& field, SampleFunction sample)
	{
		GridIterator(field.GetWidth(), field.GetHeight(), [&](int x, int y)
			{
				sample(x, y);
			}
		);
	}

	template <typename FieldType, typename SampleFunction>
	constexpr void ConstFieldGridIterator(const FieldType* field, SampleFunction sample)
	{
		GridIterator(field->GetWidth(), field->GetHeight(), [&](int x, int y)
			{
				sample(x, y);
			}
		);
	}


	// This function is used to iterate over the cells of a grid within a given radius from a specified position.
	// It calculates the bounds of the grid cells that fall within the radius and calls the provided sample function for each cell that is within the radius.
	template <typename Function>
	constexpr void ForEachCellInRadius(Float2 InCellSize, float InRadius, Float2 InPosition, Function&& SampleFunction)
	{
		int x = static_cast<int>(std::ceil(InPosition.x));
		int y = static_cast<int>(std::ceil(InPosition.y));

		int radiusX = static_cast<int>(std::ceil(InRadius / InCellSize.x));
		int radiusY = static_cast<int>(std::ceil(InRadius / InCellSize.y));

		int minX = x - radiusX;
		int maxX = x + radiusX;

		int minY = y - radiusY;
		int maxY = y + radiusY;

		float radiusSquared = InRadius * InRadius;

		for (int j = minY; j <= maxY; ++j)
		{
			for (int i = minX; i <= maxX; ++i)
			{
				float pX = (static_cast<float>(i) - InPosition.x) * InCellSize.x;
				float pY = (static_cast<float>(j) - InPosition.y) * InCellSize.y;

				// Reject if cell outside radius
				if ((pX * pX) + (pY * pY) > radiusSquared) continue;

				SampleFunction(i, j);
			}
		}
	}


	// This function fills the field with a checkerboard pattern of two values based on the block size

    template <typename T>
	constexpr void FillCheckerPattern(Field2D<T>& field, int BlockSize, T ValueA, T ValueB)
    {
		FieldGridIterator(field, [&](int x, int y)
			{
				int blockX = x / BlockSize;
				int blockY = y / BlockSize;
				if ((blockX + blockY) % 2 == 0)
				{
					field.Set(x, y, ValueA);
				}
				else
				{
					field.Set(x, y, ValueB);
				}
			}
		);
    }
}