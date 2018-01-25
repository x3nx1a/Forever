#pragma once

#include "ShaderVars.hpp"
#include "Resource.hpp"

class Texture : public Resource, public gl::DeviceObject
{
public:
	enum Format
	{
		None,
		RGBA,
		RGB,
		DXT1,
		DXT1A,
		DXT3,
		DXT5
	};
	enum Flags
	{
		Compressed = 1 << 0,
		QualityDependent = 1 << 1
	};

public:
	explicit Texture(const string& dir, const string& name, uint32_t flags = 0);
	virtual ~Texture();

	GLenum minFilter() const {
		return m_minFilter;
	}
	void setMinFilter(GLenum filter);

	GLenum magFilter() const {
		return m_magFilter;
	}
	void setMagFilter(GLenum filter);

	bool hasAlpha() const {
		return m_format != RGB && m_format != DXT1 && m_format != None;
	}

	const ivec2& size() const {
		return m_size;
	}
	uint32 flags() const {
		return m_flags;
	}

	void bind(int unit = 0) const {
		if (loaded())
			m_tex.bind(unit);
		else
			ShaderVars::blankTexture.bind(unit);
	}

protected:
	virtual void onContextLost();
	virtual void onContextRestored();
	virtual void onLoad(BinaryReader reader);

private:
	void makeFilename();

private:
	GLenum m_minFilter;
	GLenum m_magFilter;
	ivec2 m_size;
	Format m_format;
	gl::Texture2D m_tex;
	const string m_dir;
	const string m_name;
	const uint32_t m_flags;
};

typedef RefCountedPtr<Texture> TexturePtr;