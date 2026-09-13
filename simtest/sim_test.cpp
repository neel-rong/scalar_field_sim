#include "sim_test.h"


// ==========================================================================
// Core Functions
// ==========================================================================

void SimTest::Init()
{
	m_simulationTimeStep = m_simulationConfig->TargetTimeStep;
	// Get or Create the Test Output Folder
	std::filesystem::path exeDir = sim_fileOps::GetExecutableDirectory();

	m_outputFolder = exeDir / "TestOutput";

	if (!std::filesystem::exists(m_outputFolder))
	{
		std::filesystem::create_directory(m_outputFolder);
	}
}


// ==========================================================================
// Test Functions
// ==========================================================================

void SimTest::BeginTest()
{
	const char* testString[2];
	SimulationConfig::GetTestModeString(testString, 2);

	int testModeI = static_cast<int>(m_simulationConfig->TestMode);
	std::string SubFolderName = "Test";

	m_currentOutputFolderPath = sim_fileOps::CreateTimestampedOutputFolder(m_outputFolder, SubFolderName);

	std::cout << "\n-Starting Pressure Test-\n";
	std::cout << "\nDefault Thresholds are: "
		<< "\nDivergence Threshold: " << m_divergenceThreshold
		<< "\nResidual Threshold: " << m_residualThreshold << "\n";

	std::cout << "\nUse default Test Grid Resolution: ";
	GetInput(bUseDefaultTestResolution, [](int x) { return x == 0 || x == 1; }, "Invalid input. Please enter 1 for true or 0 for false.");

	if (!bUseDefaultTestResolution)
	{
		int testResCount = 0;
		std::cout << "\nEnter Test Resolution Count: ";
		GetInput(testResCount, [](int x) { return x > 0; }, "Invalid input. Please enter a positive integer.");

		for (int i = 0; i < testResCount; ++i)
		{
			int res;

			printf("\nEnter Test Reslutions %d: ", i+1);

			GetInput(res, [](int x) { return x > 0; }, "Invalid input. Please enter a positive integer.");

			m_testResolution.push_back(res);
		}
	}
	else
	{
		m_testResolution.reserve(6);

		m_testResolution.push_back(8);
		m_testResolution.push_back(16);
		m_testResolution.push_back(32);
		m_testResolution.push_back(64);
		m_testResolution.push_back(128);
		m_testResolution.push_back(256);
	}

	std::cout << "\nEnter Test Frame Count: ";
	GetInput(m_testFrameCount, [](int x) { return x > 1; }, "Invalid input. Please enter a positive integer greater than 1.");

	std::cout << "\nExport CSV File: ";
	GetInput(bExportCSV, [](bool x) {  return x == 0 || x == 1; }, "Invalid input. Please enter 1 for true or 0 for false.");
}


bool SimTest::PressureSolverTest()
{
	std::cout << "\nPressure Solver Test Started...\n";

	const char* SolverString[3];
	SimulationConfig::GetPressureSolverString(SolverString, 3);

	for (SimulationConfig::PressureSolverEnum PSE : SimulationConfig::PressureSolverArray)
	{
		m_simulationConfig->PressureSolver = PSE;
		std::string solvername = SolverString[static_cast<int>(PSE)];

		std::string pressurefilename = "PressureTest_Pressure_" + solvername;
		std::string residualfilename = "PressureTest_Residual_" + solvername;
		std::string DBPfilename = "PressureTest_DivergenceBP_" + solvername;
		std::string DAPfilename = "PressureTest_DivergenceAP_" + solvername;

		std::cout << "\nSolver Iteration for " << solvername << ": ";
		GetInput(m_pressureSolveIt, [](int x) { return x > 0; }, "Invalid input. Please enter a positive integer.");
		m_simulationConfig->SolverIterations = m_pressureSolveIt;

		std::cout << "\nStarting " << solvername << " Solver Test...\n";

		bool bthisSolverSucess = true;
		m_simulationConfig->bResetPressureField = true;

		if (bExportCSV)
		{
			m_currentPressureFilePath = sim_fileOps::CreateOutputCSVFile(pressurefilename, m_currentOutputFolderPath, false);
			if (!std::filesystem::exists(m_currentPressureFilePath)) return false;
			std::ofstream pressurefile(m_currentPressureFilePath);
			pressurefile << "Pressure Field Data\n";
			pressurefile.close();

			m_currentResidualFilePath = sim_fileOps::CreateOutputCSVFile(residualfilename, m_currentOutputFolderPath, false);
			if (!std::filesystem::exists(m_currentResidualFilePath)) return false;
			std::ofstream residualfile(m_currentResidualFilePath);
			residualfile << "Residual Field Data\n";
			residualfile.close();

			m_currentDBPFilePath = sim_fileOps::CreateOutputCSVFile(DBPfilename, m_currentOutputFolderPath, false);
			if (!std::filesystem::exists(m_currentDBPFilePath)) return false;
			std::ofstream DBPfile(m_currentDBPFilePath);
			DBPfile << "Divergence Before Projection Field Data\n";
			DBPfile.close();

			m_currentDAPFilePath = sim_fileOps::CreateOutputCSVFile(DAPfilename, m_currentOutputFolderPath, false);
			if (!std::filesystem::exists(m_currentDAPFilePath)) return false;
			std::ofstream DAPfile(m_currentDAPFilePath);
			DAPfile << "Divergence After Projection Field Data\n";
			DAPfile.close();
		}

		std::string statsfilename = "PressureTest_Stats_" + solvername;
		m_currentStatsFilePath = sim_fileOps::CreateOutputCSVFile(statsfilename, m_currentOutputFolderPath, false);
		if (!std::filesystem::exists(m_currentStatsFilePath)) return false;
		std::ofstream statsfile (m_currentStatsFilePath);
		statsfile << "Stats Data\n";
		statsfile.close();

		for (int TR : m_testResolution)
		{
			InitializeFields(Int2(TR, TR), 0.0f);

			std::string testStats = "\nResolution: " + std::to_string(TR) +
				"\nSolverIterations: " + std::to_string(m_simulationConfig->SolverIterations) + "\n";

			if (bExportCSV)
			{
				if (!sim_fileOps::AppendToFile(m_currentPressureFilePath, testStats)) return false;
				if (!sim_fileOps::AppendToFile(m_currentResidualFilePath, testStats)) return false;
				if (!sim_fileOps::AppendToFile(m_currentDBPFilePath, testStats)) return false;
				if (!sim_fileOps::AppendToFile(m_currentDAPFilePath, testStats)) return false;
			}

			if (!sim_fileOps::AppendToFile(m_currentStatsFilePath, testStats)) return false;

			m_simStepStatsCurrent.Reset();

			for (int t = 0; t < m_testFrameCount; ++t)
			{
				m_simStepStatsPrevious = m_simStepStatsCurrent;

				sim_ops::ApplySource(m_fields.VelocityField.UField(), 0.0f, 1.0f, false);
				m_fields.VelocityField.SetBoundaryNormalComponentZero();
				m_fields.VelocityField.UpdateGhostCellsNeumann();

				if (!sim_ops::ExecuteSimStep(m_fields, m_domainConfig, m_simulationConfig, m_simulationTimeStep, &m_debugFields, &m_simStepStatsCurrent))
				{
					if (bExportCSV)
					{
						if (!sim_fileOps::AppendToFile(m_currentPressureFilePath, "\nFailed To Execute Step\n")) return false;
						if (!sim_fileOps::AppendToFile(m_currentResidualFilePath, "\nFailed To Execute Step\n")) return false;
						if (!sim_fileOps::AppendToFile(m_currentDBPFilePath, "\nFailed To Execute Step\n")) return false;
						if (!sim_fileOps::AppendToFile(m_currentDAPFilePath, "\nFailed To Execute Step\n")) return false;
					}

					if (!sim_fileOps::AppendToFile(m_currentStatsFilePath, "\nFailed To Execute Step\n")) return false;

					return false;
				}

				std::string framestats = "\nFrame: " + std::to_string(t) + "\n";

				if (bExportCSV)
				{
					if (!sim_fileOps::AppendToFile(m_currentPressureFilePath, framestats)) return false;
					if (!sim_fileOps::AppendToFile(m_currentResidualFilePath, framestats)) return false;
					if (!sim_fileOps::AppendToFile(m_currentDBPFilePath, framestats)) return false;
					if (!sim_fileOps::AppendToFile(m_currentDAPFilePath, framestats)) return false;
				}

				if (!sim_fileOps::AppendToFile(m_currentStatsFilePath, framestats)) return false;

				if (bExportCSV)
				{
					if (!AppendFieldCSVToFile(m_fields.PressureField, m_currentPressureFilePath)) return false;
					if (!AppendFieldCSVToFile(m_fields.ResidualField, m_currentResidualFilePath)) return false;
					if (!AppendFieldCSVToFile(m_debugFields.DivergenceFieldBeforeProjection, m_currentDBPFilePath)) return false;
					if (!AppendFieldCSVToFile(m_debugFields.DivergenceFieldAfterProjection, m_currentDAPFilePath)) return false;
				}

				if (!AppendStatsToFile(m_simStepStatsCurrent, m_currentStatsFilePath)) return false;

				if (bExportCSV)
				{
					if (!sim_fileOps::AppendToFile(m_currentPressureFilePath, "\n")) return false;
					if (!sim_fileOps::AppendToFile(m_currentResidualFilePath, "\n")) return false;
					if (!sim_fileOps::AppendToFile(m_currentDBPFilePath, "\n")) return false;
					if (!sim_fileOps::AppendToFile(m_currentDAPFilePath, "\n")) return false;
				}

				if (!sim_fileOps::AppendToFile(m_currentStatsFilePath, "\n")) return false;


				if (t!= 0 && !CheckRMSGreater(m_simStepStatsCurrent.GetStatsDivergenceBeforeProjection(), m_simStepStatsCurrent.GetStatsDivergenceAfterProjection()))
				{
					bNumericalValidationPassed = false;
					bthisSolverSucess = false;
				}

				if (m_simStepStatsCurrent.GetStatsDivergenceAfterProjection().GetStatsRMS() > m_divergenceThreshold)
				{
					bNumericalValidationPassed = false;
					bthisSolverSucess = false;
				}

				if (t != 0 && !CheckRMSGreater(m_simStepStatsPrevious.GetStatsResidual(), m_simStepStatsCurrent.GetStatsResidual()))
				{
					bNumericalValidationPassed = false;
					bthisSolverSucess = false;
				}

				if (m_simStepStatsCurrent.GetStatsResidual().GetStatsRMS() > m_residualThreshold)
				{
					bNumericalValidationPassed = false;
					bthisSolverSucess = false;
				}
			}

			if (bthisSolverSucess)
			{
				std::cout << "\n" << solvername << " Pressure Solver Test for Resolution " << TR << " completed with SUCCESS.";
			}
			else
			{
				std::cout << "\n" << solvername << " Pressure Solver Test for Resolution " << TR << " completed with FAILURE.";
			}
		}

		std::cout << "\n";
	}

	if (bNumericalValidationPassed)
	{
		std::cout << "\nAll Pressure Solver Tests Completed with SUCCESS.";
		std::cout << "\nView Logs for Details.\n";
	}
	else
	{
		std::cout << "\nOne or more  Pressure Solver Tests Completed with FAILURE.";
		std::cout << "\nView Logs for Details.\n";
	}

	return true;
}


// =======================================================
// Helper Functions
// =======================================================

void SimTest::InitializeFields(Int2 GridResolution, float InValue)
{
	m_fields.InitializeFields(GridResolution, InValue);
	m_debugFields.InitializeFields(GridResolution, InValue);
}

bool SimTest::CheckRMSGreater(const FieldStats<float>& Stat1, const FieldStats<float>& Stat2)
{
	return (Stat1.GetStatsRMS() > Stat2.GetStatsRMS());
}

// =======================================================
// File Operation Functionss
// =======================================================

bool SimTest::AppendFieldCSVToFile(const Field2D<float>& InField, const std::filesystem::path InFilePath)
{
	if (!std::filesystem::exists(InFilePath))
	{
		printf("File does not exist: %s\n", InFilePath.string().c_str());
		return false;
	}

	std::ofstream file(InFilePath, std::ios::app);

	for (int y = 0; y < InField.GetHeight(); ++y)
	{
		for (int x = 0; x < InField.GetWidth(); ++x)
		{
			file << InField.Get(x, y);

			if (x < InField.GetWidth() - 1) file << ",";
		}
		file << "\n";
	}

	// Returning false indicates that the test could not be completed.
	// Numerical validation failures are reported through
	// bNumericalValidationPassed and the generated test output.
	return true;
}


bool SimTest::AppendStatsToFile(const SimStepStats& InSimStepStats, const std::filesystem::path InFilePath)
{
	if (!std::filesystem::exists(InFilePath)) return false;

	std::ofstream file(InFilePath, std::ios::app);

	const FieldStats<float>& VBPU = InSimStepStats.GetStatsVelocityBeforeProjection().GetStatsUField();
	const FieldStats<float>& VBPV = InSimStepStats.GetStatsVelocityBeforeProjection().GetStatsVField();

	const FieldStats<float>& VAPU = InSimStepStats.GetStatsVelocityAfterProjection().GetStatsUField();
	const FieldStats<float>& VAPV = InSimStepStats.GetStatsVelocityAfterProjection().GetStatsVField();

	const FieldStats<float>& DBP = InSimStepStats.GetStatsDivergenceBeforeProjection();
	const FieldStats<float>& DAP = InSimStepStats.GetStatsDivergenceAfterProjection();

	const FieldStats<float>& P = InSimStepStats.GetStatsPressure();
	const FieldStats<float>& R = InSimStepStats.GetStatsResidual();

	std::string VBPUString = "\nVelocity (U) Before Projection:\nMin: " + std::to_string(VBPU.GetStatsMin()) +
		"\nMax: " + std::to_string(VBPU.GetStatsMax()) +
		"\nTotal: " + std::to_string(VBPU.GetStatsTotal()) +
		"\nSum Square: " + std::to_string(VBPU.GetStatsSumSquared()) +
		"\nRMS: " + std::to_string(VBPU.GetStatsRMS()) + "\n";

	file << VBPUString;

	std::string VBPVString = "\nVelocity (V) Before Projection:\nMin: " + std::to_string(VBPV.GetStatsMin()) +
		"\nMax: " + std::to_string(VBPV.GetStatsMax()) +
		"\nTotal: " + std::to_string(VBPV.GetStatsTotal()) +
		"\nSum Square: " + std::to_string(VBPV.GetStatsSumSquared()) +
		"\nRMS: " + std::to_string(VBPV.GetStatsRMS()) + "\n";

	file << VBPVString;

	std::string DBPString = "\nDivergence Before Projection:\nMin: " + std::to_string(DBP.GetStatsMin()) +
		"\nMax: " + std::to_string(DBP.GetStatsMax()) +
		"\nTotal: " + std::to_string(DBP.GetStatsTotal()) +
		"\nSum Square: " + std::to_string(DBP.GetStatsSumSquared()) +
		"\nRMS: " + std::to_string(DBP.GetStatsRMS()) + "\n";

	file << DBPString;

	std::string PString = "\nPressure:\nMin: " + std::to_string(P.GetStatsMin()) +
		"\nMax: " + std::to_string(P.GetStatsMax()) +
		"\nTotal: " + std::to_string(P.GetStatsTotal()) +
		"\nSum Square: " + std::to_string(P.GetStatsSumSquared()) +
		"\nRMS: " + std::to_string(P.GetStatsRMS()) + "\n";

	file << PString;

	std::string RString = "\nResidual:\nMin: " + std::to_string(R.GetStatsMin()) +
		"\nMax: " + std::to_string(R.GetStatsMax()) +
		"\nTotal: " + std::to_string(R.GetStatsTotal()) +
		"\nSum Square: " + std::to_string(R.GetStatsSumSquared()) +
		"\nRMS: " + std::to_string(R.GetStatsRMS()) + "\n";

	file << RString;

	std::string DAPString = "\nDivergence After Projection:\nMin: " + std::to_string(DAP.GetStatsMin()) +
		"\nMax: " + std::to_string(DAP.GetStatsMax()) +
		"\nTotal: " + std::to_string(DAP.GetStatsTotal()) +
		"\nSum Square: " + std::to_string(DAP.GetStatsSumSquared()) +
		"\nRMS: " + std::to_string(DAP.GetStatsRMS()) + "\n";

	file << DAPString;

	std::string VAPUString = "\nVelocity (U) After Projection:\nMin: " + std::to_string(VAPU.GetStatsMin()) +
		"\nMax: " + std::to_string(VAPU.GetStatsMax()) +
		"\nTotal: " + std::to_string(VAPU.GetStatsTotal()) +
		"\nSum Square: " + std::to_string(VAPU.GetStatsSumSquared()) +
		"\nRMS: " + std::to_string(VAPU.GetStatsRMS()) + "\n";

	file << VAPUString;

	std::string VAPVString = "\nVelocity (V) After Projection:\nMin: " + std::to_string(VAPV.GetStatsMin()) +
		"\nMax: " + std::to_string(VAPV.GetStatsMax()) +
		"\nTotal: " + std::to_string(VAPV.GetStatsTotal()) +
		"\nSum Square: " + std::to_string(VAPV.GetStatsSumSquared()) +
		"\nRMS: " + std::to_string(VAPV.GetStatsRMS()) + "\n";

	file << VAPVString;

	return true;
}