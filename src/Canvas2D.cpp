#include "StdAfx.hpp"
#include "Canvas2D.hpp"
#include "Shaders.hpp"

namespace Canvas2D
{
	u8vec4 fillStyle = u8vec4(255);
	u8vec4 shadowColor = u8vec4(255);
	bool enableAlpha = true;
	float globalAlpha = 1.0f;
	int shadowBlur;

	namespace
	{
		enum PrimitiveType
		{
			Quads
		};

		struct Glyph
		{

		};

		vector<Vertex2D> s_vertices;
		PrimitiveType s_type = Quads;
		TexturePtr s_texture = nullptr;
		bool s_enableAlpha = false;
		irect s_viewRect;

		inline Vertex2D* addVertices(PrimitiveType type, int vertexCount, const TexturePtr& texture)
		{
			if (!vertexCount)
				return nullptr;

			if (s_type != type
				|| (s_texture ? s_texture->uniqueId() : 0) != (texture && texture->loaded() ? texture->uniqueId() : 0)
				|| s_enableAlpha != enableAlpha)
			{
				flush();

				s_type = type;
				s_texture = texture;
				s_enableAlpha = enableAlpha;
			}

			const std::size_t vertexIndex = s_vertices.size();
			s_vertices.resize(s_vertices.size() + (std::size_t)vertexCount);
			return &s_vertices[vertexIndex];
		}
	}

	ivec2 measureString(const string& str, int size, uint32_t flags)
	{
		const char* c = str.c_str();
		ivec2 metrics;

		while (*c)
		{
			c++;
		}

		return metrics;
	}

	void drawImage(const Image& image, const ivec2& center, float scale)
	{
		const Image::Data* const data = image.data();
		if (!data)
			return;

		Vertex2D* const v = addVertices(Quads, 4, image.texture());

		const rect rec(vec2(s_viewRect.leftTop() + center) + vec2(data->offset) * scale - vec2(data->size / 2), vec2(data->size) * scale);

		v[0].p = rec.leftTop();
		v[1].p = rec.rightTop();
		v[2].p = rec.leftBottom();
		v[3].p = rec.rightBottom();

		v[0].c = v[1].c = v[2].c = v[3].c = u8vec4(255, 255, 255, (uint8_t)(255.0f * globalAlpha));

		const rect& coords = data->coords;

		if (data->rotated)
		{
			v[0].t = coords.rightTop();
			v[1].t = coords.rightBottom();
			v[2].t = coords.leftTop();
			v[3].t = coords.leftBottom();
		}
		else
		{
			v[0].t = coords.leftTop();
			v[1].t = coords.rightTop();
			v[2].t = coords.leftBottom();
			v[3].t = coords.rightBottom();
		}
	}

	void fillRect(const irect& rect)
	{
		Vertex2D* const v = addVertices(Quads, 4, nullptr);

		const irect rec(rect.leftTop() + s_viewRect.leftTop(), rect.size());

		v[0].p = rec.leftTop();
		v[1].p = rec.rightTop();
		v[2].p = rec.leftBottom();
		v[3].p = rec.rightBottom();

		v[0].c = v[1].c = v[2].c = v[3].c = u8vec4(fillStyle.r, fillStyle.g, fillStyle.b, (uint8_t)((float)fillStyle.a * globalAlpha));
	}

	void flush()
	{
		const int vertexCount = (int)s_vertices.size();
		if (!vertexCount)
			return;

		Shaders::render2d.use();
		ShaderVars::render2dVAO.bind();

		if (s_texture)
			s_texture->bind();
		else
			ShaderVars::blankTexture.bind();

		if (s_enableAlpha)
		{
			gl::enableBlend();
			gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
			gl::disableBlend();

		gl::disableCull();
		gl::disableDepthTest();
		gl::disableDepthWrite();

		ShaderVars::render2dVBO.bind();
		ShaderVars::render2dVBO.data(vertexCount * sizeof(Vertex2D), s_vertices.data(), true);

		switch (s_type)
		{
		case Quads:
			gl::drawElements<uint16_t>(GL_TRIANGLES, (vertexCount * 6) / 4, 0);
			break;
		}

		s_texture = nullptr;
		s_vertices.clear();
	}

	void updateViewport()
	{
		const mat4 WVP = ortho((float)ShaderVars::viewport.left, (float)ShaderVars::viewport.right, (float)ShaderVars::viewport.bottom, (float)ShaderVars::viewport.top);

		Shaders::render2d.use();
		gl::uniform(Shaders::render2d.uWVP, WVP);

		glScissor(s_viewRect.left, s_viewRect.top, s_viewRect.width(), s_viewRect.height());
	}

	void setViewRect(const irect& rect)
	{
		if (s_viewRect != rect)
		{
			glScissor(s_viewRect.left, s_viewRect.top, s_viewRect.width(), s_viewRect.height());
			s_viewRect = rect;
		}
	}

	const irect& viewRect()
	{
		return s_viewRect;
	}
}