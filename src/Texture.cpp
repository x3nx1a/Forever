#include "StdAfx.hpp"
#include "Texture.hpp"
#include "Config.hpp"

Texture::Texture(const string& dir, const string& name, uint32_t flags)
	: m_minFilter(GL_NEAREST),
	m_magFilter(GL_NEAREST),
	m_size(0, 0),
	m_flags(flags),
	m_dir(dir),
	m_name(name)
{
	if (gl::isContextActive())
		onContextRestored();
}

Texture::~Texture()
{
	if (gl::isContextActive())
		onContextLost();
}

void Texture::makeFilename()
{
	string dir = m_dir;

	if (m_flags & Compressed)
	{
		dir += "texture_";

		if (m_flags & QualityDependent)
		{
			if (Config::textureQuality == 0)
				dir += "low_";
			else if (Config::textureQuality == 1)
				dir += "mid_";
		}

		dir += "dxt/";
	}

	setFilename(dir + m_name + ".bin");
}

void Texture::onContextLost()
{
	m_tex.destroy();
}

void Texture::onContextRestored()
{
	makeFilename();
	startLoad();
}

void Texture::onLoad(BinaryReader reader)
{
	if (!gl::isContextActive())
		return;

	const uint8_t ver = reader.read<uint8_t>();
	m_format = (Format)reader.read<uint8_t>();
	const int levelCount = (int)reader.read<uint8_t>();

	m_tex.create();
	m_tex.bind();

	int dataSize;
	ivec2 size;

	for (int level = 0; level < levelCount; level++)
	{
		reader >> size.x
			>> size.y
			>> dataSize;

		if (level == 0)
			m_size = size;

		char* data = new char[dataSize];
		reader.read(data, dataSize);

		switch (m_format)
		{
		case RGB:
			m_tex.image(level, GL_RGB, size.x, size.y, (const u8vec4*)data);
			break;
		case RGBA:
			m_tex.image(level, GL_RGBA, size.x, size.y, (const u8vec4*)data);
			break;
		case DXT1:
			m_tex.compressedImage(level, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, size.x, size.y, dataSize, data);
			break;
		case DXT1A:
			m_tex.compressedImage(level, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, size.x, size.y, dataSize, data);
			break;
		case DXT3:
			m_tex.compressedImage(level, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, size.x, size.y, dataSize, data);
			break;
		case DXT5:
			m_tex.compressedImage(level, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, size.x, size.y, dataSize, data);
			break;
		default:
			emscripten_log(EM_LOG_ERROR, "Unsupported texture format");
			break;
		}

		delete[] data;
	}

	m_tex.parameter(GL_TEXTURE_MIN_FILTER, m_minFilter);
	m_tex.parameter(GL_TEXTURE_MAG_FILTER, m_magFilter);
}

void Texture::setMinFilter(GLenum filter)
{
	if (filter != m_minFilter)
	{
		m_minFilter = filter;
		if (loaded())
		{
			m_tex.bind();
			m_tex.parameter(GL_TEXTURE_MIN_FILTER, filter);
		}
	}
}

void Texture::setMagFilter(GLenum filter)
{
	if (filter != m_magFilter)
	{
		m_magFilter = filter;
		if (loaded())
		{
			m_tex.bind();
			m_tex.parameter(GL_TEXTURE_MAG_FILTER, filter);
		}
	}
}