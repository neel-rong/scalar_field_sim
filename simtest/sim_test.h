#pragma once

#include <fstream>
#include <vector>
#include "field.h"
#include "simulation_operators.h"
#include "misc_operators.h"
#include "sim_properties.h"

class SimTest
{
public:

	// Constructor
	SimTest(DomainConfig* InDomainConfig, SimulationConfig* InSimConfig, SimulationTime::SimTime* InSimTime) :
		m_domainConfig(InDomainConfig),
		m_simulationConfig(InSimConfig),
		m_simTime(InSimTime) {}

	// Core Functions
	void Init();

	// Test Functions
	void BeginTest();
	bool PressureSolverTest();

private:

	// Helper Functions
	void InitializeFields(Int2 GridResolution, float InValue);
	bool CheckRMSGreater(const FieldStats<float>& Stat1, const FieldStats<float>& Stat2);


	// File Functions
	bool AppendFieldCSVToFile(const Field2D<float>& InField, const std::filesystem::path InFilePath);
	bool AppendStatsToFile(const SimStepStats& InSimStepStats, const std::filesystem::path InFilePath);


	// Configs
	SimulationConfig* m_simulationConfig = nullptr;
	DomainConfig* m_domainConfig = nullptr;
	SimulationTime::SimTime* m_simTime = nullptr;
	SimStepStats m_simStepStatsCurrent;
	SimStepStats m_simStepStatsPrevious;
	FrameProfiler m_frameProfiler;

	// Fields
	Fields m_fields;
	DebugFields m_debugFields;

	// Folder and Files
	std::filesystem::path m_outputFolder;
	std::filesystem::path m_currentOutputFolderPath;
	std::filesystem::path m_currentPressureFilePath;
	std::filesystem::path m_currentResidualFilePath;
	std::filesystem::path m_currentDBPFilePath;
	std::filesystem::path m_currentDAPFilePath;
	std::filesystem::path m_currentStatsFilePath;

	// Test Properties
	bool bUseDefaultTestResolution = false;
	std::vector<int> m_testResolution;
	int m_testFrameCount = 2;
	int m_pressureSolveIt = 40;
	bool bExportCSV = true;


	// Aggregate result of all numerical validation checks.
	// Remains false if any solver, resolution, or frame fails its
	// numerical acceptance criteria. Detailed results are exported to CSV.
	bool bNumericalValidationPassed = true;

	float m_divergenceThreshold = 0.003f;
	float m_residualThreshold = 0.003f;

	// TimeStep
	float m_simulationTimeStep = 0.01;
};