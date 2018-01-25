#include "StdAfx.hpp"
#include "ShaderVars.hpp"

#include <emscripten/html5.h>
#include <vector>

namespace gl
{
	namespace priv
	{
		bool contextActive = false;
		EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = 0;
		GLuint currentProgram;
		bool blendEnabled;
		bool depthWriteEnabled;
		bool depthTestEnabled;
		bool cullEnabled;
		GLenum currentFrontFace;
		GLenum blendFuncSrcAlpha;
		GLenum blendFuncDstAlpha;
		GLuint currentTexture[MAX_ACTIVE_TEXTURES];
		int activeTextureUnit;
		GLuint currentVertexArray;
		bool supportsVertexArray = false;
		GLuint currentVertexBuffer;
		GLuint currentIndexBuffer;
		GLuint nextVertexArrayId;
		bool vertexAttribEnabled[MAX_VERTEX_ATTRIBUTES];
		float currentLineWidth;

		class ObjectManager
		{
		public:
			static void loseAll()
			{
				vector<DeviceObject*>& objs = priv::ObjectManager::objects();
				for (auto it = objs.rbegin(); it != objs.rend(); it++)
					(*it)->onContextLost();
			}

			static void restoreAll()
			{
				vector<DeviceObject*>& objs = priv::ObjectManager::objects();
				for (auto it = objs.begin(); it != objs.end(); it++)
					(*it)->onContextRestored();
			}

			static vector<DeviceObject*>& objects()
			{
				static vector<DeviceObject*> objs;
				return objs;
			}
		};

		Shader::~Shader()
		{
			if (isContextActive())
				onContextLost();
		}

		void Shader::onContextLost()
		{
			if (m_id)
			{
				glDeleteShader(m_id);
				m_id = 0;
			}
		}

		void Shader::onContextRestored()
		{
			if (m_source.empty())
				return;

			m_id = glCreateShader(m_type);

			if (!m_id)
			{
				emscripten_log(EM_LOG_ERROR, "glCreateShader failed");
				return;
			}

			const GLchar* sources[] = {
				m_source.c_str()
			};

			glShaderSource(m_id, sizeof(sources) / sizeof(const GLchar*), sources, NULL);
			glCompileShader(m_id);

			GLint compiled;
			glGetShaderiv(m_id, GL_COMPILE_STATUS, &compiled);

			if (!compiled)
			{
				GLint infoLen = 0;
				glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);

				if (infoLen > 1)
				{
					char* infoLog = new char[infoLen];

					glGetShaderInfoLog(m_id, infoLen, NULL, infoLog);
					emscripten_log(EM_LOG_ERROR, "Error compiling shader: %s", infoLog);

					delete[] infoLog;
				}

				glDeleteShader(m_id);
				m_id = 0;
			}
		}

		Buffer::~Buffer()
		{
			destroy();
		}

		void Buffer::create()
		{
			if (!m_id)
			{
				glGenBuffers(1, &m_id);
				if (!m_id)
					emscripten_log(EM_LOG_ERROR, "glGenBuffers failed");
			}
		}
	}

	bool createContext(const char* target)
	{
		EmscriptenWebGLContextAttributes attributes;
		emscripten_webgl_init_context_attributes(&attributes);

		attributes.alpha = false;
		attributes.depth = true;
		attributes.stencil = false;
		attributes.antialias = false;
		attributes.premultipliedAlpha = false;
		attributes.preserveDrawingBuffer = false;
		attributes.preferLowPowerToHighPerformance = false;
		attributes.failIfMajorPerformanceCaveat = false; // wrong support on some platforms
		attributes.majorVersion = 1;
		attributes.minorVersion = 0;
		attributes.enableExtensionsByDefault = false;

		priv::context = emscripten_webgl_create_context(target, &attributes);

		if (priv::context > 0)
		{
			emscripten_webgl_make_context_current(priv::context);
			return true;
		}
		else
			return false;
	}

	void destroyContext()
	{
		if (priv::context > 0)
		{
			emscripten_webgl_destroy_context(priv::context);
			priv::context = 0;
		}
	}

	bool restoreContext()
	{
		if (priv::context <= 0)
			return false;
		if (priv::contextActive)
			return true;

		emscripten_webgl_enable_extension(priv::context, "WEBGL_compressed_texture_s3tc");
		priv::supportsVertexArray = emscripten_webgl_enable_extension(priv::context, "OES_vertex_array_object") != 0;

		priv::currentProgram = 0;
		glUseProgram(0);

		glDepthFunc(GL_LEQUAL);
		glCullFace(GL_FRONT);

		priv::cullEnabled = false;
		glDisable(GL_CULL_FACE);

		priv::blendEnabled = false;
		glDisable(GL_BLEND);

		priv::depthTestEnabled = false;
		glDisable(GL_DEPTH_TEST);

		priv::depthWriteEnabled = false;
		glDepthMask(GL_FALSE);

		priv::currentFrontFace = GL_CCW;
		glFrontFace(GL_CCW);

		priv::blendFuncSrcAlpha = GL_SRC_ALPHA;
		priv::blendFuncDstAlpha = GL_ONE_MINUS_SRC_ALPHA;
		glBlendFunc(priv::blendFuncSrcAlpha, priv::blendFuncDstAlpha);

		for (int unit = 0; unit < MAX_ACTIVE_TEXTURES; unit++)
		{
			priv::currentTexture[unit] = 0;
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		priv::activeTextureUnit = 0;
		glActiveTexture(GL_TEXTURE0);

		priv::currentVertexBuffer = 0;
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		priv::currentVertexArray = 0;

		if (priv::supportsVertexArray)
		{
			glBindVertexArrayOES(0);
		}
		else
		{
			priv::currentIndexBuffer = 0;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			priv::vertexAttribEnabled[0] = true;
			glEnableVertexAttribArray(0);

			for (GLuint idx = 1; idx < MAX_VERTEX_ATTRIBUTES; idx++)
			{
				priv::vertexAttribEnabled[idx] = false;
				glDisableVertexAttribArray(idx);
			}

			priv::nextVertexArrayId = 1;
		}

		priv::currentLineWidth = 1.0f;
		glLineWidth(1.0f);

		priv::contextActive = true;

		ShaderVars::initAll();

		priv::ObjectManager::restoreAll();

		return true;
	}

	void loseContext()
	{
		if (!priv::contextActive)
			return;

		priv::contextActive = false;
		priv::ObjectManager::loseAll();

		ShaderVars::releaseAll();

		priv::currentProgram = 0;
		priv::cullEnabled = false;
		priv::blendEnabled = false;
		priv::depthTestEnabled = false;
		priv::depthWriteEnabled = false;
		priv::currentFrontFace = GL_CCW;
		priv::blendFuncSrcAlpha = GL_SRC_ALPHA;
		priv::blendFuncDstAlpha = GL_ONE_MINUS_SRC_ALPHA;
		for (int unit = 0; unit < MAX_ACTIVE_TEXTURES; unit++)
			priv::currentTexture[unit] = 0;
		priv::activeTextureUnit = 0;
		priv::currentVertexArray = 0;
		priv::currentVertexBuffer = 0;
		priv::currentIndexBuffer = 0;
		priv::vertexAttribEnabled[0] = true;
		for (GLuint idx = 1; idx < MAX_VERTEX_ATTRIBUTES; idx++)
			priv::vertexAttribEnabled[idx] = false;
		priv::currentLineWidth = 1.0f;
	}

	DeviceObject::DeviceObject()
	{
		vector<DeviceObject*>& objs = priv::ObjectManager::objects();
		objs.push_back(this);
	}

	DeviceObject::~DeviceObject()
	{
		vector<DeviceObject*>& objs = priv::ObjectManager::objects();
		auto it = find(objs.begin(), objs.end(), this);
		if (it != objs.end())
			objs.erase(it);
	}

	Program:: ~Program()
	{
		if (isContextActive())
			onContextLost();
	}

	void Program::onContextLost()
	{
		if (m_id)
		{
			if (priv::currentProgram == m_id)
			{
				glUseProgram(0);
				priv::currentProgram = 0;
			}

			glDeleteProgram(m_id);
			m_id = 0;
		}
	}

	void Program::onContextRestored()
	{
		if (!m_vertex || !m_fragment)
			return;

		static const char* attribNames[] = {
			"aPos",
			"aTexCoord0",
			"aTexCoord1",
			"aDiffuse",
			"aNormal",
			"aWeight",
			"aBoneId"
		};

		m_id = glCreateProgram();

		if (!m_id)
		{
			emscripten_log(EM_LOG_ERROR, "glCreateProgram failed");
			return;
		}

		glAttachShader(m_id, m_vertex->id());
		glAttachShader(m_id, m_fragment->id());

		for (GLuint i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
			if (m_attribute[i])
				glBindAttribLocation(m_id, i, attribNames[i]);

		glLinkProgram(m_id);

		glDetachShader(m_id, m_vertex->id());
		glDetachShader(m_id, m_fragment->id());

		GLint linked;
		glGetProgramiv(m_id, GL_LINK_STATUS, &linked);

		if (!linked)
		{
			GLint infoLen = 0;
			glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);

			if (infoLen > 1)
			{
				char* infoLog = new char[infoLen];

				glGetProgramInfoLog(m_id, infoLen, NULL, infoLog);
				emscripten_log(EM_LOG_ERROR, "Error linking program: %s", infoLog);

				delete[] infoLog;
			}

			glDeleteProgram(m_id);
			m_id = 0;
		}
	}

	void Texture2D::create()
	{
		if (!m_id)
		{
			glGenTextures(1, &m_id);
			if (!m_id)
				emscripten_log(EM_LOG_ERROR, "glGenTextures failed");
		}
	}

	void VertexArray::create()
	{
		if (!m_id)
		{
			if (priv::supportsVertexArray)
			{
				glGenVertexArraysOES(1, &m_id);
				if (!m_id)
					emscripten_log(EM_LOG_ERROR, "glGenVertexArrays failed");
			}
			else
			{
				m_id = priv::nextVertexArrayId;
				priv::nextVertexArrayId++;
			}
		}
	}
}