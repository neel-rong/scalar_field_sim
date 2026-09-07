#pragma once

#include "field.h"
#include "sim_properties.h"
#include "debug_helpers.h"
#include "simulation_operators.h"
#include "misc_operators.h"




class Simulation
{

public:

	//Constructor
	Simulation()
	{
	}

	Simulation(DomainConfig* InDomainConfig, SimulationConfig* InConfigs, RenderConfig* InRenderConfig, FrameProfiler* InFrameProfiler, MouseState* InMouseState, Playback* InPlayback) :
		m_domainConfig(InDomainConfig), m_simulationConfig(InConfigs), m_renderConfig(InRenderConfig), m_frameProfiler(InFrameProfiler), m_mouseState(InMouseState), m_playback(InPlayback)
	{
	}

	// ====================================================================================
	// Core Functions
	// ====================================================================================

	void Init();
	void Reset();
	void Update();

	// ====================================================================================
	// Simulation Functions
	// ====================================================================================

	int RunSimulation();


	// Field Pointers
	Field2D<float>* m_scalarRenderField = &m_fields.PressureField;
	MACGridVelocityField2D* m_velocityRenderField = &m_fields.VelocityField;

	// Render Properties
	float m_pixelValueMin = std::numeric_limits<float>::max();
	float m_pixelValueMax = std::numeric_limits<float>::lowest();

	Float2 m_velocityMin = Float2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Float2 m_velocityMax = Float2(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

private:

	// Configurations and External States
	Playback* m_playback = nullptr;
	SimulationConfig* m_simulationConfig = nullptr;
	RenderConfig* m_renderConfig = nullptr;
	MouseState* m_mouseState = nullptr;
	FrameProfiler* m_frameProfiler = nullptr;
	DomainConfig* m_domainConfig = nullptr;

	// Emitters
	CircularEmitter<Field2D<float>> m_dyeEmitter;
	CircularEmitter<Field2D<float>> m_velocityUEmitter;
	CircularEmitter<Field2D<float>> m_velocityVEmitter;

	// Fields
	Fields m_fields;

	void UpdatePixelMinMaxValue(const Field2D<float>* Field);
	void UpdateVelocityMinMaxValue(const MACGridVelocityField2D* Field);
};