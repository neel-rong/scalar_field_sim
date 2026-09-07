#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <cmath>
#include "field.h"
#include "grid_algorithms.h"

struct DomainConfig
{
	// ======================================
	// Constructors
	// ======================================
	DomainConfig() : GridResolution(128, 128), DomainSize(128.0f, 128.0f) {}
	DomainConfig(Int2 InGridResolution, Float2 InDomainSize) :
		GridResolution(InGridResolution), DomainSize(InDomainSize) {}

	// ======================================
	// Members
	// ======================================
	// Grid Properties
	// Physical simulation resolution.
	// Does not include ghost cells.
	Int2 GridResolution = Int2(128, 128);		// Default grid resolution

	// Domain Size in physical units.
	// This is the size of the simulation domain in physical space.
	// The grid resolution and domain size together determine the physical size of each grid cell.
	// The domain size is used to convert between grid coordinates and physical coordinates.
	// Ideal DomainSize should maintain the same aspect ratio as GridResolution.
	Float2 DomainSize = Float2(128.0f, 128.0f);		// Default domain Size


	// ======================================
	// Functions
	// ======================================

	Float2 GetCellSize() const
	{
		return Float2(DomainSize.x / GridResolution.x, DomainSize.y / GridResolution.y);
	}
};




// =============================================================================================================================
// Properties for dye simulation, including diffusion, advection, and vorticity confinement parameters
// =============================================================================================================================

struct DyeProperties
{
	// Constructors
	DyeProperties()
	{ }

	DyeProperties(float InDiffusionCoefficient, float InDiffusionDecayRate, float InAdvectionRate, float InVorticityConfinementScale) :
		DiffusionCoefficient(InDiffusionCoefficient),
		DiffusionDecayRate(InDiffusionDecayRate),
		AdvectionRate(InAdvectionRate),
		VorticityConfinementScale(InVorticityConfinementScale)
	{ }

	// Simulation Properties
	float DiffusionCoefficient = 0.1f;
	float DiffusionDecayRate = 0.1f;
	float AdvectionRate = 0.1f;
	float VorticityConfinementScale = 0.1f;
};


// =============================================================================================================================
// Emitter Properties
// =============================================================================================================================

// The emitter currently operates on scalar fields.
// The template is retained from the earlier generic field implementation.
template<ArithmeticField T>
struct EmitterProperties
{
	// Constructors
	EmitterProperties() :
		Position(Float2(0.0f, 0.0f)),
		Radius(0.0f),
		InjectionValueAverage(typename T::ValueType{}),
		InjectionValueVariance(typename T::ValueType{}),
		BlendTime(0.0f)
	{ }

	EmitterProperties(Float2 InPosition, float InRadius, typename T::ValueType InAverageValue, typename T::ValueType InVariance, float InBlendTime) :
		Position(InPosition),
		Radius(InRadius),
		InjectionValueAverage(InAverageValue),
		InjectionValueVariance(InVariance),
		BlendTime(InBlendTime)
	{ }

	// Emitter Properties
	Float2 Position;
	float Radius;

	typename T::ValueType InjectionValueAverage;
	typename T::ValueType InjectionValueVariance;

	float BlendTime;
};


// =====================================================
// Simulation Config
// =====================================================
// NOTE: Do not use this for computations and calculations.
// Simulation Config is the purpose of UI representation and process UI input.
// The actual parameters inside the simulation may hide logic or parameters.
// Use simulation or field classes for computations.
struct SimulationConfig
{
	SimulationConfig() { }

	SimulationConfig(
		EmitterProperties<Field2D<float>>& InDensityEmitterProperties,
		EmitterProperties<Field2D<float>>& InVelocityUEmitterProperties,
		EmitterProperties<Field2D<float>>& InVelocityVEmitterProperties,
		DyeProperties& InDyeProperties) :
	DensityEmitterProperties(InDensityEmitterProperties),
	VelocityUEmitterProperties(InVelocityUEmitterProperties),
	VelocityVEmitterProperties(InVelocityVEmitterProperties),
	Dye(InDyeProperties) { }

	// Enums
	enum class SimulationModeEnum : uint8_t
	{
		Test,
		Simulation
	};

	enum class TestModeEnum : uint8_t
	{
		DivergenceTest,
		PressureTest
	};

	enum class PressureSolverEnum : uint8_t
	{
		Jacobi,
		GaussSeidel,
		RedBlackGaussSeidel
	};

	static constexpr std::array<PressureSolverEnum, 3> PressureSolverArray =
	{	PressureSolverEnum::Jacobi,
		PressureSolverEnum::GaussSeidel,
		PressureSolverEnum::RedBlackGaussSeidel
	};

	// Simulation Mode
	SimulationModeEnum SimulationMode = SimulationModeEnum::Simulation;

	// Simulation Properties
	float TimeStep = 0.025f;			//Default time step for simulation
	float DiffusionScale = 0.0f;
	float VorticityScale= 0.0f;
	float AdvectionScale = 1.0f;

	// Emitter Properties
	EmitterProperties<Field2D<float>> DensityEmitterProperties;
	EmitterProperties<Field2D<float>> VelocityUEmitterProperties;
	EmitterProperties<Field2D<float>> VelocityVEmitterProperties;

	// Dye properties
	DyeProperties Dye;


	// Solver Properties
	PressureSolverEnum PressureSolver = PressureSolverEnum::RedBlackGaussSeidel;
	bool bResetPressureField = false;
	int SolverIterations = 200;													// Default Pressure Solver Iterations
	float OverRelaxation = 1.97f;												// Keep over relaxation value between 1 - 2

	float VelocityMouseInjectionScale = 0.25f;
	float MouseInjectionRadius = 6.0f;

	// Test Properties
	TestModeEnum TestMode = TestModeEnum::DivergenceTest;

	static void GetSimulationModeString(const char** OutStrings, const size_t InSize)
	{
		assert (InSize >= 2);	// Ensure the output array has enough space

		OutStrings[0] = "Test";
		OutStrings[1] = "Simulation";
	}

	static void GetTestModeString(const char** OutStrings, const size_t InSize)
	{
		assert (InSize >= 2);	// Ensure the output array has enough space

		OutStrings[0] = "Divergence Test";
		OutStrings[1] = "Pressure Test";
	}

	static void GetPressureSolverString(const char** OutStrings, const size_t InSize)
	{
		assert (InSize >= 3);	// Ensure the output array has enough space

		OutStrings[0] = "Jacobi";
		OutStrings[1] = "Gauss Seidel";
		OutStrings[2] = "Red-Black Gauss Seidel";
	}
};


// =====================================================
// Render Configs
// =====================================================
struct RenderConfig
{
	// Visualization Modes
	enum class ScalarVisualizationMode : uint8_t
	{
		DensityField,
		PressureField,
		VelocityDivergenceField,
	};

	enum class VelocityVisualizationMode : uint8_t
	{
		NONE,
		CellCenteredVelocityField,
		FaceCenteredVelocityField,
	};

	// Constructor
	RenderConfig(){} // Default Constructor

	// Properties
	ScalarVisualizationMode ScalarMode = ScalarVisualizationMode::DensityField;
	VelocityVisualizationMode VelocityMode = VelocityVisualizationMode::NONE;
	Int2 RenderResolution = Int2(720, 720);

	// Show Debug Grids
	bool bShowGrid = false;

	static void GetScalarVisualizationModeString(const char** OutStrings, size_t InSize)
	{
		assert (InSize >= 3); // Ensure the output array has enough space
		OutStrings[0] = "Density Field";
		OutStrings[1] = "Pressure Field";
		OutStrings[2] = "Velocity Divergence Field";
	}

	static void GetVelocityVisualizationModeString(const char** OutStrings, size_t InSize)
	{
		assert (InSize >= 3); // Ensure the output array has enough space
		OutStrings[0] = "NONE";
		OutStrings[1] = "Cell Centered Velocity Field";
		OutStrings[2] = "Face Centered Velocity Field";
	}
};


// Playback States
enum Playback : uint8_t
{
	Stop,
	Play,
	Pause,
	Reset
};


// ====================================================
// Emitters
// ====================================================

// The emitter currently operates on scalar fields.
// The template is retained from the earlier generic field implementation.
template<ArithmeticField T>
struct CircularEmitter
{
public:

	CircularEmitter() :
		Properties(),
		ElapsedTime(0.0f),
		PreviousValue{},
		TargetValue{}
	{ }

	void Init(EmitterProperties<T>* InProperties)
	{
		Properties = InProperties;
		ElapsedTime = 0.0f;
		PreviousValue = {};
		TargetValue = GetRandomValue(Properties->InjectionValueAverage, Properties->InjectionValueVariance);
	}



	// Reset
	void Reset()
	{
		ElapsedTime = 0.0f;
		PreviousValue = {};
		TargetValue = GetRandomValue(Properties->InjectionValueAverage, Properties->InjectionValueVariance);
	}

	// Injects a random value within AverageCellValue and +/- CellValueVariance in the field at InjectionPosition within Radius
	void Inject(T& Field, Float2 InCellSize, float dt)
	{
		Update(dt);

		float Alpha;

		if (Properties->BlendTime > 0.0f)
		{
			Alpha = std::clamp(ElapsedTime / Properties->BlendTime, 0.0f, 1.0f);
		}
		else
		{
			Alpha = 1.0f; // If BlendTime is zero, set Alpha to 1 to immediately apply the target value
		}

		// Blend the current value with the target value based on the elapsed time and blend time
		typename T::ValueType BlendedValue = Lerp(PreviousValue, TargetValue, Alpha);

		Float2 pos = Float2(static_cast<float>(Properties->Position.x) / InCellSize.x, static_cast<float>(Properties->Position.y) / InCellSize.y);

		grid_algorithms::ForEachCellInRadius(InCellSize, Properties->Radius, pos, [&](int x, int y)
			{
				if (x >= 0 && x < Field.GetWidth() && y >= 0 && y < Field.GetHeight())
				{
					Field.Set(x, y, Field.Get(x, y) + BlendedValue);
				}
			});
	}

	// Updates the elapsed time and generates a new target value if the blend time has been exceeded
	void Update(float dt)
	{
		ElapsedTime += dt; // Simulate time progression for each injection

		if (ElapsedTime > Properties->BlendTime)
		{
			PreviousValue = TargetValue;
			TargetValue = GetRandomValue(Properties->InjectionValueAverage, Properties->InjectionValueVariance);

			ElapsedTime = 0.0f; // Reset elapsed time after injection
		}
	}

	// Getter for EmitterProperties
	EmitterProperties<T>& GetProperties()
	{
		return *Properties;
	}

	const EmitterProperties<T>& GetProperties() const
	{
		return *Properties;
	}



private:

	EmitterProperties<T>* Properties;

	float ElapsedTime;
	typename T::ValueType PreviousValue = typename T::ValueType{};
	typename T::ValueType TargetValue = typename T::ValueType{};


	std::mt19937 rng{ std::random_device{}() };

	float GetRandomValue(float Average, float Variance)
	{
		return GetRandomFloat(Average - Variance, Average + Variance);
	}

	Float2 GetRandomValue(Float2 Average, Float2 Variance)
	{
		float ValueX = GetRandomFloat((Average.x - Variance.x), (Average.x + Variance.x));
		float ValueY = GetRandomFloat((Average.y - Variance.y), (Average.y + Variance.y));

		return Float2(ValueX, ValueY);
	}

	float GetRandomFloat(float min, float max)
	{
		std::uniform_real_distribution<float> Rand{ min, max };
		return Rand(rng);
	}

	float Lerp(float InPreviousValue, float TargetValue, float InAlpha)
	{
		return std::lerp(InPreviousValue, TargetValue, InAlpha);
	}

	Float2 Lerp(Float2 InPreviousValue, Float2 TargetValue, float InAlpha)
	{
		return Float2(std::lerp(InPreviousValue.x, TargetValue.x, InAlpha),
					  std::lerp(InPreviousValue.y, TargetValue.y, InAlpha));
	}
};