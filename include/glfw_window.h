#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "misc_operators.h"


class Window
{
public:

	// Constructor

	Window(int width, int height, const char* title) : width(width), height(height)
	{
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW");
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Create the Window and set context
		m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (!m_window)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwMaximizeWindow(m_window);
		glfwMakeContextCurrent(m_window);

		// Load OpenGL functions using GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			glfwDestroyWindow(m_window);
			m_window = nullptr;
			glfwTerminate();
			throw std::runtime_error("Failed to initialize GLAD");
		}

		std::cout << "OpenGL Version: "
			<< glGetString(GL_VERSION)
			<< "\n";


		glfwSwapInterval(0);
	}


	// Destructor
	~Window()
	{
		if (m_window)
			glfwDestroyWindow(m_window);

		glfwTerminate();
	}

	// Event Functions

	bool ShouldClose() const
	{
		return glfwWindowShouldClose(m_window);
	}

	void PollEvents() const
	{
		glfwPollEvents();
	}

	void SwapBuffers() const
	{
		glfwSwapBuffers(m_window);
	}

	GLFWwindow* GetNativeWindow()
	{
		return m_window;
	}

	// Profiler Function
	void Update(FrameProfiler& InFrameProfiler)
	{
		// Set Current Time to glfw Time
		InFrameProfiler.CurrentTime = glfwGetTime();

		// Calculate Delta Time (Current Time - Previous Time)
		InFrameProfiler.DeltaTime = InFrameProfiler.CurrentTime - InFrameProfiler.PreviousTime;

		// Set Previous Time = Current Time
		InFrameProfiler.PreviousTime = InFrameProfiler.CurrentTime;

		// Increment FrameCount
		InFrameProfiler.FrameCount++;


		// Calucalte FPS from Delta Time
		if (InFrameProfiler.DeltaTime > 0)
		{
			InFrameProfiler.FPS = 1 / InFrameProfiler.DeltaTime;
		}
	}


private:
	// GLFW Window
	GLFWwindow* m_window = nullptr;

	int width = 0;
	int height = 0;
};