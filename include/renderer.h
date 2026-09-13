#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include "field.h"
#include "grid_algorithms.h"
#include "sim_properties.h"
#include "misc_operators.h"



// ===========================================================
// Shader Class
// ===========================================================

class Shader
{
public:

	// Constructor
	Shader() {}

	// Destructor
	~Shader()
	{
		if (m_program != 0)
		{
			glDeleteProgram(m_program);
		}
	}


	// Functions

	bool Load(const std::string& vertexShader, const std::string& fragmentShader);
	void Bind();
	void Unbind();


	GLuint GetProgramID() const
	{
		return m_program;
	}


private:

	GLuint CompileShader(GLenum InTarget, const std::string& InShaderSource);


	GLuint m_program = 0;
};


// ===========================================================
// Line Vertex
// ===========================================================

struct LineVertex
{
	Float2 Position = Float2(0.0f, 0.0f);
	Float4 Color = Float4(0.0f, 0.0f, 0.0f, 1.0f);
};

// ===========================================================
// Dynamic Line Renderer
// ===========================================================

class DynamicLineRenderer
{
public:

	// Constructor
	DynamicLineRenderer() {};

	// Destructor
	~DynamicLineRenderer()
	{
		if (m_vao != 0)
		{
			glDeleteVertexArrays(1, &m_vao);
		}
		if (m_vbo != 0)
		{
			glDeleteBuffers(1, &m_vbo);
		}
	};


	bool Init();

	void BeginLine();

	void AddLine(Float2 A, Float2 B, Float4 Color);

	void End();

private:

	std::vector<LineVertex> m_vertices;
	Shader m_shader;

	GLuint m_vao = 0;
	GLuint m_vbo = 0;
};


// ===========================================================
// Mesh Renderer
// ===========================================================

class MeshRenderer
{
public:

	// Constructor
	MeshRenderer() {};

	// Destructor
	~MeshRenderer()
	{
		if (m_vao != 0)
		{
			glDeleteVertexArrays(1, &m_vao);
		}
		if (m_vbo != 0)
		{
			glDeleteBuffers(1, &m_vbo);
		}
	};

	bool Init();

	void BeginFrame();

	void DrawMesh();

private:

	float m_vertices[12] = {
		// Triangle 1
	   -1.0f,-1.0f,
		1.0f,-1.0f,
	   -1.0f, 1.0f,

	   // Triangle 2
	   -1.0f, 1.0f,
		1.0f,-1.0f,
		1.0f, 1.0f
	};

	GLuint m_vao = 0;
	GLuint m_vbo = 0;
};



// ===========================================================
// Render Layout
// ===========================================================

struct RenderLayout
{
	int fieldWidth = 0;
	int fieldHeight = 0;

	int framebufferWidth = 0;
	int framebufferHeight = 0;

	Float2 renderRegion = Float2(0.0f, 0.0f);
	float cellSize = 0;

	float offsetX = 0;
	float offsetY = 0;

	float gridPixelWidth = 0;
	float gridPixelHeight = 0;
};




// ==============================================================
// Renderer
// ==============================================================

class Renderer
{

public:

	// Constructor
	Renderer()
	{
	};

	Renderer(RenderConfig* InRenderConfig, SimulationConfig* InSimulationConfig, Field2D<float>* InScalarField, MACGridVelocityField2D* InMacVelocityField) :
		m_renderConfig(InRenderConfig),
		m_sim_config(InSimulationConfig),
		m_scalarField(InScalarField),
		m_macVelocityField(InMacVelocityField)
	{
	}

	// Destructor
	~Renderer()
	{
		if (m_fbo != 0)
		{
			glDeleteFramebuffers(1, &m_fbo);
		}
		if (m_outTexture != 0)
		{
			glDeleteTextures(1, &m_outTexture);
		}
		if (m_fieldTexture != 0)
		{
			glDeleteTextures(1, &m_fieldTexture);
		}
	}



	// Render Config
	RenderConfig* m_renderConfig = nullptr;
	SimulationConfig* m_sim_config = nullptr;


	// ====================================
    // Public Functions
	// ====================================
    bool Init();
	void Reset();


    // Update Scalar Field
	void UpdateFields(Field2D<float>* InFieldScalar, MACGridVelocityField2D* InFieldVector)
	{
		m_scalarField = InFieldScalar;
		m_macVelocityField = InFieldVector;
	}

	// Render function

	void ComputeLayout();

	Float2 GridToPixel(Float2 InGridPosition);
	Float2 PixelToGrid(Float2 InPixel);

	Float2 PixelToNDC(Float2 InPixel);
	Float2 NDCToPixel(Float2 InNDC);

	Float2 GridToNDC(Float2 InGridPosition);
	Float2 NDCToGrid(Float2 InNDC);

	void BeginFrame();

	void DrawDebugGrid();

	void DrawVectorField(float HeadLength, float HeadWidth, float InVelocityScale = 1.0f);

	void DrawFrame();

	void Update(float InPixelMin, float InPixelMax, Float2 InVectorMin, Float2 InVectorMax);


	// Getter Functions
	GLuint GetTexID()
	{
		return m_outTexture;
	}

private:

	float m_marginFraction = 0.05f;	// Margin fraction for rendering

	// Field Pointer
    Field2D<float>* m_scalarField = nullptr;
	MACGridVelocityField2D* m_macVelocityField = nullptr;

	float m_pixelMin = 0.0f;
	float m_pixelMax = 1.0f;

	Float2 m_vectorMin = Float2(0.0f, 0.0f);
	Float2 m_vectorMax = Float2(1.0f, 1.0f);

	RenderLayout m_renderLayout;


	// OpenGL Object IDs
	GLuint m_fbo = 0;
	GLuint m_fieldTexture = 0;
	GLuint m_outTexture = 0;

	// OpenGL Uniform Locations
	GLint m_fieldTexUniLocation = 0;
	GLint m_pixelMinMaxUniLocation = 0;
	GLint m_marginFractionUniLocation = 0;

	// Renderer and Shaders
	DynamicLineRenderer m_lineRenderer;
	MeshRenderer m_meshRenderer;
	Shader m_shader;

	// Shader attributes and uniforms
	Float4 m_backgroundColor = Float4(0.08f, 0.08f, 0.08f, 1.0f);
	Float4 m_velocityLineColor = Float4(0.0f, 0.5f, 0.5f, 1.0f);
	Float4 m_gridLineColor = Float4(0.5f, 0.5f, 0.5f, 1.0f);


	// ====================================
	// Private Functions
	// ====================================

	// Helper Functions

	bool InitializeFboAndTextures();
	void BindFieldTexture();

};
