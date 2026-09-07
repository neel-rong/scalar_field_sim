#pragma once
#include <limits>
#include<conio.h>
#include "sim_test.h"
#include "misc_operators.h"


int main()
{
	DomainConfig m_DomainConfig;
	SimulationConfig m_testSimConfig;
	SimulationTime::SimTime m_testSimTime;

	SimTest Test(&m_DomainConfig , &m_testSimConfig, &m_testSimTime);
	Test.Init();

	Test.BeginTest();
	if (!Test.PressureSolverTest())
	{
		std::cout << "\nPressure Solver Test Failed with error";
		std::cout << "\nView Logs for Details.\n";
		return -1;
	}

	std::cout << "\nPress any key to exit..." << std::endl;

	getch();

	return 0;
}