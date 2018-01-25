#pragma once

#include <GLES2/gl2.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

namespace gl
{
	static const int MAX_ACTIVE_TEXTURES = 2;
	static const GLuint MAX_VERTEX_ATTRIBUTES = 7;

	namespace priv
	{
		extern bool contextActive;
		extern GLuint currentProgram;
		extern bool blendEnabled;
		extern bool depthWriteEnabled;
		extern bool depthTestEnabled;
		extern bool cullEnabled;
		extern GLenum currentFrontFace;
		extern GLenum blendFuncSrcAlpha;
		extern GLenum blendFuncDstAlpha;
		extern GLuint currentTexture[MAX_ACTIVE_TEXTURES];
		extern int activeTextureUnit;
		extern GLuint currentVertexArray;
		extern bool supportsVertexArray;
		extern GLuint currentVertexBuffer;
		extern GLuint currentIndexBuffer;
		extern GLuint nextVertexArrayId;
		extern bool vertexAttribEnabled[MAX_VERTEX_ATTRIBUTES];
		extern float currentLineWidth;

		class ObjectManager;
		class Buffer;
		class Shader;
	}

	class DeviceObject;
	class VertexShader;
	class FragmentShader;
	class VertexBuffer;
	class IndexBuffer;
	class VertexArray;
	class Texture2D;

	bool createContext(const char* target);
	bool restoreContext();
	void loseContext();
	void destroyContext();

	inline bool isContextActive()
	{
		return priv::contextActive;
	}

	class DeviceObject
	{
	protected:
		DeviceObject();

	public:
		virtual ~DeviceObject();

	protected:
		virtual void onContextLost() = 0;
		virtual void onContextRestored() = 0;

	private:
		DeviceObject(const DeviceObject&) = delete;
		DeviceObject& operator=(const DeviceObject&) = delete;

		friend class priv::ObjectManager;
	};

	namespace priv
	{
		template<typename T> inline GLenum typeOf() { return 0; }
		template<> inline GLenum typeOf<u8vec4>() { return GL_UNSIGNED_BYTE; }
		template<> inline GLenum typeOf<u8vec3>() { return GL_UNSIGNED_BYTE; }
		template<> inline GLenum typeOf<u8vec2>() { return GL_UNSIGNED_BYTE; }
		template<> inline GLenum typeOf<uint8_t>() { return GL_UNSIGNED_BYTE; }
		template<> inline GLenum typeOf<u16vec4>() { return GL_UNSIGNED_SHORT; }
		template<> inline GLenum typeOf<u16vec3>() { return GL_UNSIGNED_SHORT; }
		template<> inline GLenum typeOf<u16vec2>() { return GL_UNSIGNED_SHORT; }
		template<> inline GLenum typeOf<uint16_t>() { return GL_UNSIGNED_SHORT; }
		template<> inline GLenum typeOf<i8vec4>() { return GL_BYTE; }
		template<> inline GLenum typeOf<i8vec3>() { return GL_BYTE; }
		template<> inline GLenum typeOf<i8vec2>() { return GL_BYTE; }
		template<> inline GLenum typeOf<int8_t>() { return GL_BYTE; }
		template<> inline GLenum typeOf<i16vec4>() { return GL_SHORT; }
		template<> inline GLenum typeOf<i16vec3>() { return GL_SHORT; }
		template<> inline GLenum typeOf<i16vec2>() { return GL_SHORT; }
		template<> inline GLenum typeOf<int16_t>() { return GL_SHORT; }
		template<> inline GLenum typeOf<vec4>() { return GL_FLOAT; }
		template<> inline GLenum typeOf<vec3>() { return GL_FLOAT; }
		template<> inline GLenum typeOf<vec2>() { return GL_FLOAT; }
		template<> inline GLenum typeOf<float>() { return GL_FLOAT; }
		template<typename T> inline GLenum sizeOf() { return 0; }
		template<> inline GLenum sizeOf<u8vec4>() { return 4; }
		template<> inline GLenum sizeOf<u8vec3>() { return 3; }
		template<> inline GLenum sizeOf<u8vec2>() { return 2; }
		template<> inline GLenum sizeOf<uint8_t>() { return 1; }
		template<> inline GLenum sizeOf<u16vec4>() { return 4; }
		template<> inline GLenum sizeOf<u16vec3>() { return 3; }
		template<> inline GLenum sizeOf<u16vec2>() { return 2; }
		template<> inline GLenum sizeOf<uint16_t>() { return 1; }
		template<> inline GLenum sizeOf<i8vec4>() { return 4; }
		template<> inline GLenum sizeOf<i8vec3>() { return 3; }
		template<> inline GLenum sizeOf<i8vec2>() { return 2; }
		template<> inline GLenum sizeOf<int8_t>() { return 1; }
		template<> inline GLenum sizeOf<i16vec4>() { return 4; }
		template<> inline GLenum sizeOf<i16vec3>() { return 3; }
		template<> inline GLenum sizeOf<i16vec2>() { return 2; }
		template<> inline GLenum sizeOf<int16_t>() { return 1; }
		template<> inline GLenum sizeOf<vec4>() { return 4; }
		template<> inline GLenum sizeOf<vec3>() { return 3; }
		template<> inline GLenum sizeOf<vec2>() { return 2; }
		template<> inline GLenum sizeOf<float>() { return 1; }

		class Shader : public DeviceObject
		{
		public:
			Shader(GLenum type)
				: m_type(type),
				m_id(0)
			{
			}
			virtual ~Shader();

			void setSource(const string& source)
			{
				m_source = source;
				if (isContextActive())
					onContextRestored();
			}

			GLuint id() const
			{
				return m_id;
			}

		protected:
			virtual void onContextLost();
			virtual void onContextRestored();

		private:
			GLuint m_id;
			GLenum m_type;
			string m_source;
		};

		class Buffer
		{
		public:
			Buffer(GLenum type)
				:m_id(0),
				m_type(type)
			{
			}
			virtual ~Buffer();

			void destroy()
			{
				if (m_id)
				{
					if (m_type == GL_ARRAY_BUFFER)
					{
						if (priv::currentVertexBuffer == m_id)
						{
							glBindBuffer(GL_ARRAY_BUFFER, 0);
							priv::currentVertexBuffer = 0;
						}
					}

					glDeleteBuffers(1, &m_id);
					m_id = 0;
				}
			}

			void create();

			void data(GLsizeiptr size, const GLvoid* data, bool stream = false)
			{
				glBufferData(m_type, size, data, stream ? GL_STREAM_DRAW : GL_STATIC_DRAW);
			}

		protected:
			GLuint m_id;
			GLenum m_type;

		private:
			Buffer(const Buffer&) = delete;
			Buffer& operator=(const Buffer&) = delete;
		};
	}

	class FragmentShader : public priv::Shader
	{
	public:
		FragmentShader()
			: Shader(GL_FRAGMENT_SHADER)
		{
		}
	};

	class VertexShader : public priv::Shader
	{
	public:
		VertexShader()
			: Shader(GL_VERTEX_SHADER)
		{
		}
	};

	class Program : public DeviceObject
	{
	public:
		Program()
			: m_id(0),
			m_fragment(nullptr),
			m_vertex(nullptr)
		{
			for (GLuint i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
				m_attribute[i] = false;
		}
		virtual ~Program();

		template<std::size_t AttribCount> void link(const FragmentShader* fragment, const VertexShader* vertex, const GLuint(&attribs)[AttribCount])
		{
			m_fragment = fragment;
			m_vertex = vertex;

			for (std::size_t i = 0; i < AttribCount; i++)
				m_attribute[attribs[i]] = true;

			if (isContextActive())
				onContextRestored();
		}

		void use()
		{
			if (priv::currentProgram != m_id)
			{
				glUseProgram(m_id);
				priv::currentProgram = m_id;
			}
		}

		int location(const char* uniform) const
		{
			return glGetUniformLocation(m_id, uniform);
		}

	protected:
		virtual void onContextLost();
		virtual void onContextRestored();

	private:
		GLuint m_id;
		const FragmentShader* m_fragment;
		const VertexShader* m_vertex;
		bool m_attribute[MAX_VERTEX_ATTRIBUTES];
	};

	class Texture2D
	{
	public:
		Texture2D()
			: m_id(0)
		{
		}
		~Texture2D()
		{
			destroy();
		}

		void destroy()
		{
			if (m_id)
			{
				for (int unit = 0; unit < MAX_ACTIVE_TEXTURES; unit++)
				{
					if (priv::currentTexture[unit] == m_id)
					{
						if (priv::activeTextureUnit != unit)
						{
							glActiveTexture(GL_TEXTURE0 + unit);
							priv::activeTextureUnit = unit;
						}

						glBindTexture(GL_TEXTURE_2D, 0);
						priv::currentTexture[unit] = 0;
					}
				}

				glDeleteTextures(1, &m_id);
				m_id = 0;
			}
		}

		void create();

		void bind(int unit = 0) const
		{
			if (priv::activeTextureUnit != unit)
			{
				glActiveTexture(GL_TEXTURE0 + unit);
				priv::activeTextureUnit = unit;
			}

			if (priv::currentTexture[unit] != m_id)
			{
				glBindTexture(GL_TEXTURE_2D, m_id);
				priv::currentTexture[unit] = m_id;
			}
		}

		template<typename T> void image(GLint level, GLint format, GLsizei width, GLsizei height, const T* pixels)
		{
			if (priv::currentTexture[priv::activeTextureUnit] != m_id)
			{
				glBindTexture(GL_TEXTURE_2D, m_id);
				priv::currentTexture[priv::activeTextureUnit] = m_id;
			}

			glTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, format, priv::typeOf<T>(), pixels);
		}

		void compressedImage(GLint level, GLint format, GLsizei width, GLsizei height, GLsizei imageSize, const GLvoid* data)
		{
			if (priv::currentTexture[priv::activeTextureUnit] != m_id)
			{
				glBindTexture(GL_TEXTURE_2D, m_id);
				priv::currentTexture[priv::activeTextureUnit] = m_id;
			}

			glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, imageSize, data);
		}

		void parameter(GLenum pname, GLenum param)
		{
			if (priv::currentTexture[priv::activeTextureUnit] != m_id)
			{
				glBindTexture(GL_TEXTURE_2D, m_id);
				priv::currentTexture[priv::activeTextureUnit] = m_id;
			}

			glTexParameteri(GL_TEXTURE_2D, pname, param);
		}

		void parameterf(GLenum pname, GLfloat param)
		{
			if (priv::currentTexture[priv::activeTextureUnit] != m_id)
			{
				glBindTexture(GL_TEXTURE_2D, m_id);
				priv::currentTexture[priv::activeTextureUnit] = m_id;
			}

			glTexParameterf(GL_TEXTURE_2D, pname, param);
		}

	private:
		GLuint m_id;

	private:
		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
	};

	class VertexArray
	{
	private:
		struct AttribPointer
		{
			GLuint VBO;
			GLint size;
			GLenum type;
			GLboolean normalized;
			GLsizei stride;
			const GLvoid* offset;
		};

	public:
		VertexArray()
			: m_id(0),
			m_IBO(0)
		{
			for (GLuint idx = 0; idx < MAX_VERTEX_ATTRIBUTES; idx++)
				m_attribs[idx].VBO = 0;
		}
		~VertexArray()
		{
			destroy();
		}

		void destroy()
		{
			if (m_id)
			{
				if (priv::supportsVertexArray)
				{
					if (priv::currentVertexArray == m_id)
					{
						glBindVertexArrayOES(0);
						priv::currentVertexArray = 0;
					}

					glDeleteVertexArraysOES(1, &m_id);
				}
				else
				{
					if (priv::currentVertexArray == m_id)
						priv::currentVertexArray = 0;

					for (GLuint idx = 0; idx < MAX_VERTEX_ATTRIBUTES; idx++)
						m_attribs[idx].VBO = 0;
				}

				m_id = 0;
				m_IBO = 0;
			}
		}

		void create();

		void bind() const
		{
			if (priv::currentVertexArray != m_id)
			{
				if (priv::supportsVertexArray)
					glBindVertexArrayOES(m_id);
				else
				{
					if (m_IBO && priv::currentIndexBuffer != m_IBO)
					{
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
						priv::currentIndexBuffer = m_IBO;
					}

					for (GLuint idx = 0; idx < MAX_VERTEX_ATTRIBUTES; idx++)
					{
						const AttribPointer& attrib = m_attribs[idx];

						if (attrib.VBO)
						{
							if (!priv::vertexAttribEnabled[idx])
							{
								glEnableVertexAttribArray(idx);
								priv::vertexAttribEnabled[idx] = true;
							}

							if (priv::currentVertexBuffer != attrib.VBO)
							{
								glBindBuffer(GL_ARRAY_BUFFER, attrib.VBO);
								priv::currentVertexBuffer = attrib.VBO;
							}

							glVertexAttribPointer(idx, attrib.size, attrib.type, attrib.normalized, attrib.stride, attrib.offset);
						}
						else if (priv::vertexAttribEnabled[idx])
						{
							glDisableVertexAttribArray(idx);
							priv::vertexAttribEnabled[idx] = false;
						}
					}
				}

				priv::currentVertexArray = m_id;
			}
		}

		template<typename T> void vertexAttribPointer(GLuint idx, bool normalized, GLsizei stride, uint32_t offset)
		{
			if (priv::supportsVertexArray)
			{
				glEnableVertexAttribArray(idx);
				glVertexAttribPointer(idx, priv::sizeOf<T>(), priv::typeOf<T>(), normalized ? GL_TRUE : GL_FALSE, stride, (const GLvoid*)offset);
			}
			else
			{
				AttribPointer& attrib = m_attribs[idx];
				attrib.VBO = priv::currentVertexBuffer;
				attrib.size = priv::sizeOf<T>();
				attrib.type = priv::typeOf<T>();
				attrib.normalized = normalized ? GL_TRUE : GL_FALSE;
				attrib.stride = stride;
				attrib.offset = (const GLvoid*)offset;

				if (!priv::vertexAttribEnabled[idx])
				{
					glEnableVertexAttribArray(idx);
					priv::vertexAttribEnabled[idx] = true;
				}

				glVertexAttribPointer(idx, attrib.size, attrib.type, attrib.normalized, attrib.stride, attrib.offset);
			}
		}

	private:
		GLuint m_id;
		GLuint m_IBO;
		AttribPointer m_attribs[MAX_VERTEX_ATTRIBUTES];

	private:
		VertexArray(const VertexArray&) = delete;
		VertexArray& operator=(const VertexArray&) = delete;

		friend class IndexBuffer;
	};

	class VertexBuffer : public priv::Buffer
	{
	public:
		VertexBuffer()
			: Buffer(GL_ARRAY_BUFFER)
		{
		}

		void bind() const
		{
			if (priv::currentVertexBuffer != m_id)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m_id);
				priv::currentVertexBuffer = m_id;
			}
		}
	};

	class IndexBuffer : public priv::Buffer
	{
	public:
		IndexBuffer()
			: Buffer(GL_ELEMENT_ARRAY_BUFFER)
		{
		}

		void bind(VertexArray& vertexArray) const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
			priv::currentIndexBuffer = m_id;

			vertexArray.m_IBO = m_id;
		}
	};

	inline void enableCull()
	{
		if (!priv::cullEnabled)
		{
			glEnable(GL_CULL_FACE);
			priv::cullEnabled = true;
		}
	}

	inline void disableCull()
	{
		if (priv::cullEnabled)
		{
			glDisable(GL_CULL_FACE);
			priv::cullEnabled = false;
		}
	}

	inline void enableBlend()
	{
		if (!priv::blendEnabled)
		{
			glEnable(GL_BLEND);
			priv::blendEnabled = true;
		}
	}

	inline void disableBlend()
	{
		if (priv::blendEnabled)
		{
			glDisable(GL_BLEND);
			priv::blendEnabled = false;
		}
	}

	inline void enableDepthTest()
	{
		if (!priv::depthTestEnabled)
		{
			glEnable(GL_DEPTH_TEST);
			priv::depthTestEnabled = true;
		}
	}

	inline void disableDepthTest()
	{
		if (priv::depthTestEnabled)
		{
			glDisable(GL_DEPTH_TEST);
			priv::depthTestEnabled = false;
		}
	}

	inline void enableDepthWrite()
	{
		if (!priv::depthWriteEnabled)
		{
			glDepthMask(GL_TRUE);
			priv::depthWriteEnabled = true;
		}
	}

	inline void disableDepthWrite()
	{
		if (priv::depthWriteEnabled)
		{
			glDepthMask(GL_FALSE);
			priv::depthWriteEnabled = false;
		}
	}

	inline void frontFace(GLenum frontFace)
	{
		if (priv::currentFrontFace != frontFace)
		{
			glFrontFace(frontFace);
			priv::currentFrontFace = frontFace;
		}
	}

	inline void blendFunc(GLenum srcAlpha, GLenum dstAlpha)
	{
		if (srcAlpha != priv::blendFuncSrcAlpha || dstAlpha != priv::blendFuncDstAlpha)
		{
			glBlendFunc(srcAlpha, dstAlpha);
			priv::blendFuncSrcAlpha = srcAlpha;
			priv::blendFuncDstAlpha = dstAlpha;
		}
	}

	inline void lineWidth(float width)
	{
		if (priv::currentLineWidth != width)
		{
			glLineWidth(width);
			priv::currentLineWidth = width;
		}
	}

	template<typename T> inline void drawElements(GLenum mode, GLsizei count, uint32_t offset)
	{
		glDrawElements(mode, count, priv::typeOf<T>(), (const GLvoid*)offset);
	}

	inline void drawArrays(GLenum mode, GLint first, GLsizei count)
	{
		glDrawArrays(mode, first, count);
	}

	inline void uniform(int l, const float& v) { glUniform1f(l, v); }
	inline void uniform(int l, const int& v) { glUniform1i(l, v); }
	inline void uniform(int l, const vec2& v) { glUniform2fv(l, 1, (const GLfloat*)&v); }
	inline void uniform(int l, const ivec2& v) { glUniform2iv(l, 1, (const GLint*)&v); }
	inline void uniform(int l, const vec3& v) { glUniform3fv(l, 1, (const GLfloat*)&v); }
	inline void uniform(int l, const ivec3& v) { glUniform3iv(l, 1, (const GLint*)&v); }
	inline void uniform(int l, const vec4& v) { glUniform4fv(l, 1, (const GLfloat*)&v); }
	inline void uniform(int l, const ivec4& v) { glUniform4iv(l, 1, (const GLint*)&v); }
	inline void uniform(int l, const mat2& v) { glUniformMatrix2fv(l, 1, GL_FALSE, (const GLfloat*)&v); }
	inline void uniform(int l, const mat3& v) { glUniformMatrix3fv(l, 1, GL_FALSE, (const GLfloat*)&v); }
	inline void uniform(int l, const mat4& v) { glUniformMatrix4fv(l, 1, GL_FALSE, (const GLfloat*)&v); }

	inline void uniform(int l, GLsizei c, const float* v) { glUniform1fv(l, c, v); }
	inline void uniform(int l, GLsizei c, const int* v) { glUniform1iv(l, c, v); }
	inline void uniform(int l, GLsizei c, const vec2* v) { glUniform2fv(l, c, (const GLfloat*)v); }
	inline void uniform(int l, GLsizei c, const ivec2* v) { glUniform2iv(l, c, (const GLint*)v); }
	inline void uniform(int l, GLsizei c, const vec3* v) { glUniform3fv(l, c, (const GLfloat*)v); }
	inline void uniform(int l, GLsizei c, const ivec3* v) { glUniform3iv(l, c, (const GLint*)v); }
	inline void uniform(int l, GLsizei c, const vec4* v) { glUniform4fv(l, c, (const GLfloat*)v); }
	inline void uniform(int l, GLsizei c, const ivec4* v) { glUniform4iv(l, c, (const GLint*)v); }
	inline void uniform(int l, GLsizei c, const mat2* v) { glUniformMatrix2fv(l, c, GL_FALSE, (const GLfloat*)v); }
	inline void uniform(int l, GLsizei c, const mat3* v) { glUniformMatrix3fv(l, c, GL_FALSE, (const GLfloat*)v); }
	inline void uniform(int l, GLsizei c, const mat4* v) { glUniformMatrix4fv(l, c, GL_FALSE, (const GLfloat*)v); }
}