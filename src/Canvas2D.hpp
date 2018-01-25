#pragma once

#include "Vertex.hpp"
#include "Image.hpp"

namespace Canvas2D
{
	enum TextFlags
	{
		Bold = 1 << 0,
		Underline = 1 << 1,
		Stroke = 1 << 2,
		Italic = 1 << 3,
		Highlight = 1 << 4
	};

	ivec2 measureString(const string& str, int size, uint32_t flags = 0);
	//void renderString(const ivec2& pos, const string& str, int size, const u8vec4& color, const u8vec4& outlineColor = u8vec4(0));

	void drawImage(const Image& image, const ivec2& pos, float scale = 1.0f);

	void fillRect(const irect& rect);

	void updateViewport();
	void setViewRect(const irect& rect);
	const irect& viewRect();
	void flush();

	extern u8vec4 fillStyle;
	extern u8vec4 shadowColor;
	extern int shadowBlur;
	extern bool enableAlpha;
	extern float globalAlpha;
}