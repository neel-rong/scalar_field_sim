#include "renderer.h"



// ==========================================================================================================================================
// Shader Class
// ==========================================================================================================================================

bool Shader::Load(const std::string& InVertexShader, const std::string& InFragmentShader)
{
	m_program = glCreateProgram();
	GLuint vs = CompileShader(GL_VERTEX_SHADER, InVertexShader);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, InFragmentShader);

	if (vs == 0 || fs == 0)
	{
		if (vs != 0) glDeleteShader(vs);
		if (fs != 0) glDeleteShader(fs);
		glDeleteProgram(m_program);
		m_program = 0;
		return false;
	}

	glAttachShader(m_program, vs);
	glAttachShader(m_program, fs);
	glLinkProgram(m_program);

	GLint success;
	glGetProgramiv(m_program, GL_LINK_STATUS, &success);

	if (!success)
	{
		char info[1024];
		glGetProgramInfoLog(m_program, 1024, nullptr, info);
		std::cout << info << std::endl;

		if (vs != 0) glDeleteShader(vs);
		if (fs != 0) glDeleteShader(fs);
		glDeleteProgram(m_program);
		m_program = 0;

		return false;
	}


	glValidateProgram(m_program);

	GLint validateStatus;
	glGetProgramiv(m_program, GL_VALIDATE_STATUS, &validateStatus);

	if (!validateStatus)
	{
		char info[1024];
		glGetProgramInfoLog(m_program, 1024, nullptr, info);
		std::cout << info << std::endl;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	return true;
}


GLuint Shader::CompileShader(GLenum InTarget, const std::string& InSource)
{
	GLuint id = glCreateShader(InTarget);
	const char* src = InSource.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	GLint success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		char info[1024];
		glGetShaderInfoLog(id, 1024, nullptr, info);
		std::cout << info << std::endl;

		glDeleteShader(id);
		return 0;
	}

	return id;
}


void Shader::Bind()
{
	glUseProgram(m_program);
}


void Shader::Unbind()
{
	glUseProgram(0);
}


// ==========================================================================================================================================
// Mesh Renderer Class
// ==========================================================================================================================================

bool MeshRenderer::Init()
{
	// Generate Vertex Array
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Generate Vertex Buffer
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	// Define Vertex Attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Float2), (void*)0);

	// Update Vertex Buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);

	// Unbind VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

void MeshRenderer::BeginFrame()
{
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
}

void MeshRenderer::DrawMesh()
{
	// Draw Elements
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

// ==========================================================================================================================================
// Renderer Class
// ==========================================================================================================================================

bool Renderer::Init()
{

	// Compute Render Layout
	ComputeLayout();

	// Create Framebuffer, Quads, and Shaders for the Renderer

	if (!InitializeFboAndTextures())
	{
		return false;
	}


	BindFieldTexture();

	// ===========================================================
	// Create Shader Program

	// Read Shader Files
	std::string vertexShader = sim_fileOps::ReadFile("shaders/meshShader.vert");
	std::string fragmentShader = sim_fileOps::ReadFile("shaders/meshShader.frag");

	// Create Shader Program
	if (!m_shader.Load(vertexShader, fragmentShader))
	{
		return false;
	}

	m_shader.Bind();

	m_fieldTexUniLocation = glGetUniformLocation(m_shader.GetProgramID(), "u_FieldTexture");
	m_pixelMinMaxUniLocation = glGetUniformLocation(m_shader.GetProgramID(), "u_PixelMinMax");

	glUniform1i(m_fieldTexUniLocation, 0);
	glUniform2f(m_pixelMinMaxUniLocation, m_pixelMin, m_pixelMax);

	m_shader.Unbind();


	if (!m_meshRenderer.Init())
	{
		return false;
	}

	// Initialize Line Renderer
	if (!m_lineRenderer.Init())
	{
		return false;
	}

	return true;
}


void Renderer::Reset()
{
	ComputeLayout();

	glBindTexture(GL_TEXTURE_2D, m_outTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_renderLayout.framebufferWidth, m_renderLayout.framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, m_fieldTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_renderLayout.fieldWidth, m_renderLayout.fieldHeight, 0, GL_RED, GL_FLOAT, nullptr);
}



void Renderer::BeginFrame()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glViewport(0, 0, m_renderLayout.framebufferWidth, m_renderLayout.framebufferHeight);

	glClearColor(m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, m_backgroundColor.w);
	glClear(GL_COLOR_BUFFER_BIT);

	m_meshRenderer.BeginFrame();
}

void Renderer::DrawFrame()
{

	BindFieldTexture();
	m_shader.Bind();

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_scalarField->GetWidth(), m_scalarField->GetHeight(), GL_RED, GL_FLOAT, m_scalarField->GetData().data());

	glUniform2f(m_pixelMinMaxUniLocation, m_pixelMin, m_pixelMax);

	m_meshRenderer.DrawMesh();

	m_lineRenderer.BeginLine();

	if (m_renderConfig->bShowGrid)
	{
		DrawDebugGrid();
	}

	if (m_renderConfig->VelocityMode != RenderConfig::VelocityVisualizationMode::NONE)
	{
		float headLength = std::max(std::abs(m_vectorMin.x + m_vectorMax.x) / 2, std::abs(m_vectorMin.y + m_vectorMax.y) / 2.0f) / 2.0f;
		DrawVectorField(headLength, headLength/4, 0.05f);
	}

	m_lineRenderer.End();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


bool Renderer::InitializeFboAndTextures()
{
	glGenFramebuffers(1, &m_fbo);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_outTexture);

	glBindTexture(GL_TEXTURE_2D, m_outTexture);


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_renderLayout.framebufferWidth, m_renderLayout.framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);


	// Create, bind, and allocate memory for field texture
	glGenTextures(1, &m_fieldTexture);

	glBindTexture(GL_TEXTURE_2D, m_fieldTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_renderLayout.fieldWidth, m_renderLayout.fieldHeight, 0, GL_RED, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}



// Helper Functions

void Renderer::ComputeLayout()
{
	m_renderLayout.fieldWidth = m_scalarField->GetWidth();
	m_renderLayout.fieldHeight = m_scalarField->GetHeight();

	m_renderLayout.framebufferWidth = m_renderConfig->RenderResolution.x;
	m_renderLayout.framebufferHeight = m_renderConfig->RenderResolution.y;

	m_renderLayout.renderRegion = Float2(m_renderLayout.framebufferWidth, m_renderLayout.framebufferHeight) * 0.95f;

	m_renderLayout.cellSize = std::min(m_renderLayout.renderRegion.x / m_renderLayout.fieldWidth,
		m_renderLayout.renderRegion.y / m_renderLayout.fieldHeight);

	m_renderLayout.gridPixelWidth = m_renderLayout.fieldWidth * m_renderLayout.cellSize;
	m_renderLayout.gridPixelHeight = m_renderLayout.fieldHeight * m_renderLayout.cellSize;

	m_renderLayout.offsetX = (m_renderLayout.framebufferWidth - m_renderLayout.gridPixelWidth) * 0.5f;
	m_renderLayout.offsetY = (m_renderLayout.framebufferHeight - m_renderLayout.gridPixelHeight) * 0.5f;
}

void Renderer::BindFieldTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fieldTexture);
}


// Converter Functions ==================================================

Float2 Renderer::GridToPixel(Float2 InGridPosition)
{
	return Float2(m_renderLayout.offsetX + InGridPosition.x * m_renderLayout.cellSize,
		m_renderLayout.offsetY + InGridPosition.y * m_renderLayout.cellSize
	);
}


Float2 Renderer::PixelToGrid(Float2 InPixel)
{
	return Float2((InPixel.x - m_renderLayout.offsetX) / m_renderLayout.cellSize,
		(InPixel.y - m_renderLayout.offsetY) / m_renderLayout.cellSize
	);
}


Float2 Renderer::PixelToNDC(Float2 InPixel)
{
	return Float2(
		InPixel.x / m_renderLayout.framebufferWidth * 2.0f - 1.0f,
		InPixel.y / m_renderLayout.framebufferHeight * 2.0f - 1.0f
		);
}


Float2 Renderer::NDCToPixel(Float2 InNDC)
{
	return Float2(
		(InNDC.x + 1.0f) * 0.5f * m_renderLayout.framebufferWidth,
		(InNDC.y + 1.0f) * 0.5f * m_renderLayout.framebufferHeight
	);
}


Float2 Renderer::GridToNDC(Float2 InGridPosition)
{
	return PixelToNDC(GridToPixel(InGridPosition));
}

Float2 Renderer::NDCToGrid(Float2 InNDC)
{
	return PixelToGrid(NDCToPixel(InNDC));
}


void Renderer::DrawVectorField(float HeadLength, float HeadWidth, float InVelocityScale)
{
	Field2D<float>& U = m_macVelocityField->UField();
	Field2D<float>& V = m_macVelocityField->VField();

	Float2 offsetU = U.GetSamplingOffset();
	Float2 offsetV = V.GetSamplingOffset();

	int padding = Field2D<float>::GhostCellPadding;

	auto DrawArrow = [&](Float2 InStart, Float2 InEnd, Float2 InLeft, Float2 InRight)
		{
			m_lineRenderer.AddLine(InStart, InEnd, m_velocityLineColor);
			m_lineRenderer.AddLine(InEnd, InLeft, m_velocityLineColor);
			m_lineRenderer.AddLine(InEnd, InRight, m_velocityLineColor);
		};

	if (m_renderConfig->VelocityMode == RenderConfig::VelocityVisualizationMode::CellCenteredVelocityField)
	{
		// When using Cell Center Visualization, use the MacVelocity field width() and Height(), for clarity,
		// although a MacGrid with the same resolution as a Field2D<float> returns the same width and height,
		// the staggered indices is hidden in the UField and the VField, instead.
		// The physical MacGrid represents the same physical space as that of a Field2D<float> field.
		for (int x = 0; x < m_macVelocityField->GetWidth(); ++x)
		{
			for (int y = 0; y < m_macVelocityField->GetHeight(); ++y)
			{
				Float2 Start = m_macVelocityField->GetCellCenterPosition(x, y);
				Float2 value = m_macVelocityField->GetVelocityAtCellCenter(x, y);
				if (value.LengthSquared() < 1e-8f) continue;
				Float2 dir = value.Normalized();
				Float2 End = Start + value * InVelocityScale;
				Float2 perp = Float2(-dir.y, dir.x);
				Float2 left = End - dir * HeadLength * InVelocityScale + perp * HeadWidth * InVelocityScale;
				Float2 right = End - dir * HeadLength * InVelocityScale - perp * HeadWidth * InVelocityScale;
				DrawArrow(GridToNDC(Start), GridToNDC(End), GridToNDC(left), GridToNDC(right));
			}
		}
	}

	// When using Face Center Visualization, use the UField and VField of the actual MacGrid.
	// The UField and VField of a MacGrid has sample offset- U: Float2(0.0f, 0.5f) , V: Float2(0.5f, 0.0f).
	// Therefore, for proper mapping use the sampling offset therein, for the purpose of rendering.
	// MACGrid like a Field<float> field, has a Ghost Cell Padding of 1 cell, and that needs to be taken into account during iteration.
	else if (m_renderConfig->VelocityMode == RenderConfig::VelocityVisualizationMode::FaceCenteredVelocityField)
	{
		for (int x = padding; x < U.GetWidth() - padding; ++x)
		{
			for (int y = padding; y < U.GetHeight() - padding; ++y)
			{
				Float2 Start = Float2(x + offsetU.x, y + offsetU.y);
				Float2 value = Float2(U.Get(x, y), 0.0f);

				if (value.LengthSquared() < 1e-8f) continue;

				Float2 dir = value.Normalized();
				Float2 End = Start + value * InVelocityScale;

				Float2 perp = Float2(-dir.y, dir.x);

				Float2 left = End - dir * HeadLength * InVelocityScale + perp * HeadWidth * InVelocityScale;
				Float2 right = End - dir * HeadLength * InVelocityScale - perp * HeadWidth * InVelocityScale;

				DrawArrow(GridToNDC(Start), GridToNDC(End), GridToNDC(left), GridToNDC(right));
			}
		}

		for (int x = padding; x < V.GetWidth() - padding; ++x)
		{
			for (int y = padding; y < V.GetHeight() - padding; ++y)
			{
				Float2 Start = Float2(x + offsetV.x, y + offsetV.y);

				Float2 value = Float2(0.0f, V.Get(x, y));

				if (value.LengthSquared() < 1e-8f) continue;

				Float2 dir = value.Normalized();
				Float2 End = Start + value * InVelocityScale;

				Float2 perp = Float2(-dir.y, dir.x);

				Float2 left = End - dir * HeadLength * InVelocityScale + perp * HeadWidth * InVelocityScale;
				Float2 right = End - dir * HeadLength * InVelocityScale - perp * HeadWidth * InVelocityScale;

				DrawArrow(GridToNDC(Start), GridToNDC(End), GridToNDC(left), GridToNDC(right));
			}
		}
	}
}


void Renderer::DrawDebugGrid()
{

	for (int x = 0; x <= m_renderLayout.fieldWidth; ++x)
	{
		Float2 Start = Float2(x, 0);
		Float2 End = Float2(x, m_renderLayout.fieldHeight);
		m_lineRenderer.AddLine(GridToNDC(Start), GridToNDC(End), m_gridLineColor);
	}
	for (int y = 0; y <= m_renderLayout.fieldHeight; ++y)
	{
		Float2 Start = Float2(0, y);
		Float2 End = Float2(m_renderLayout.fieldWidth, y);
		m_lineRenderer.AddLine(GridToNDC(Start), GridToNDC(End), m_gridLineColor);
	}
}


void Renderer::Update(float InPixelMin, float InPixelMax, Float2 InVectorMin, Float2 InVectorMax)
{
	m_pixelMin = InPixelMin;
	m_pixelMax = InPixelMax;

	m_vectorMin = InVectorMin;
	m_vectorMax = InVectorMax;
}


// =======================================================================================================================================
// Dynamic Line Renderer
// =======================================================================================================================================

bool DynamicLineRenderer::Init()
{
	// Create Vertex Array Object and Vertex Buffer Object for Dynamic Line Rendering

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, Position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, Color));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// =================================================
	// Create Shader Program for Dynamic Line Rendering

	// Read Shader Source Code from Files
	std::string vertexShaderSource = sim_fileOps::ReadFile("shaders/lineShader.vert");
	std::string fragmentShaderSource = sim_fileOps::ReadFile("shaders/lineShader.frag");

	// Create Shader and Link Shader Program
	return m_shader.Load(vertexShaderSource, fragmentShaderSource);
}

void DynamicLineRenderer::BeginLine()
{
	m_vertices.clear();

	m_shader.Bind();
}


void DynamicLineRenderer::AddLine(Float2 A, Float2 B, Float4 Color)
{
	m_vertices.push_back({A, Color});
	m_vertices.push_back({B, Color});
}

void DynamicLineRenderer::End()
{
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertex) * m_vertices.size(), m_vertices.data(), GL_DYNAMIC_DRAW);

	glDrawArrays(GL_LINES, 0, m_vertices.size());
}
