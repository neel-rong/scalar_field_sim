#pragma once

template<typename T>
requires std::is_arithmetic_v<T>
struct FieldStats
{
	FieldStats()
	{
		Reset();
	}

	void Reset()
	{
		sample_count = 0;
		min = std::numeric_limits<T>::max();
		max = std::numeric_limits<T>::lowest();
		total = T{};
		sum_squared = T{};
		RMS = T{};
	}

	void Update(const T& value)
	{
		sample_count++;
		min = std::min(min, value);
		max = std::max(max, value);
		total += value;
		sum_squared += value * value;
		RMS = std::sqrt(sum_squared / sample_count);
	}

	T GetStatsMin() const
	{
		return	min;
	}

	T GetStatsMax() const
	{
		return max;
	}

	T GetStatsTotal() const
	{
		return total;
	}

	T GetStatsSumSquared() const
	{
		return sum_squared;
	}

	T GetStatsRMS() const
	{
		return RMS;
	}

private:

	unsigned int sample_count;
	T min;
	T max;
	T total;
	T sum_squared;
	T RMS;
};



// ====================================
// MACGrid Velocity Stats
// ====================================

struct MACGridVelocityStats
{
	MACGridVelocityStats()
	{
		Reset();
	}

	void Reset()
	{
		UFieldStats.Reset();
		VFieldStats.Reset();
	}

	FieldStats<float>& GetStatsUField()
	{
		return UFieldStats;
	}

	const FieldStats<float>& GetStatsUField() const
	{
		return UFieldStats;
	}

	FieldStats<float>& GetStatsVField()
	{
		return VFieldStats;
	}

	const FieldStats<float>& GetStatsVField() const
	{
		return VFieldStats;
	}

private:

	FieldStats<float> UFieldStats;
	FieldStats<float> VFieldStats;
};


// ====================================
// Sim Step Stats
// ====================================

struct SimStepStats
{
	void Reset()
	{
		DivergenceBeforeProjectionStats.Reset();
		DivergenceAfterProjectionStats.Reset();
		PressureStats.Reset();
		ResidualStats.Reset();
		VelocityStatsBeforeProjection.Reset();
		VelocityStatsAfterProjection.Reset();
	}

	FieldStats<float>& GetStatsDivergenceBeforeProjection()
	{
		return DivergenceBeforeProjectionStats;
	}

	const FieldStats<float>& GetStatsDivergenceBeforeProjection() const
	{
		return DivergenceBeforeProjectionStats;
	}

	FieldStats<float>& GetStatsDivergenceAfterProjection()
	{
		return DivergenceAfterProjectionStats;
	}

	const FieldStats<float>& GetStatsDivergenceAfterProjection() const
	{
		return DivergenceAfterProjectionStats;
	}

	FieldStats<float>& GetStatsPressure()
	{
		return PressureStats;
	}

	const FieldStats<float>& GetStatsPressure() const
	{
		return PressureStats;
	}

	FieldStats<float>& GetStatsResidual()
	{
		return ResidualStats;
	}

	const FieldStats<float>& GetStatsResidual() const
	{
		return ResidualStats;
	}

	MACGridVelocityStats& GetStatsVelocityBeforeProjection()
	{
		return VelocityStatsBeforeProjection;
	}

	const MACGridVelocityStats& GetStatsVelocityBeforeProjection() const
	{
		return VelocityStatsBeforeProjection;
	}

	MACGridVelocityStats& GetStatsVelocityAfterProjection()
	{
		return VelocityStatsAfterProjection;
	}

	const MACGridVelocityStats& GetStatsVelocityAfterProjection() const
	{
		return VelocityStatsAfterProjection;
	}


private:

	FieldStats<float> DivergenceBeforeProjectionStats;
	FieldStats<float> DivergenceAfterProjectionStats;
	FieldStats<float> PressureStats;
	FieldStats<float> ResidualStats;

	MACGridVelocityStats VelocityStatsBeforeProjection;
	MACGridVelocityStats VelocityStatsAfterProjection;
};