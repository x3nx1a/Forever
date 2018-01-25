#pragma once

#include "Texture.hpp"
#include "BinaryReader.hpp"

class Image
{
public:
	struct Data
	{
		const string* textureName;
		ivec2 offset;
		ivec2 size;
		rect coords;
		bool rotated;
	};

public:
	explicit Image(const Data* data);

	explicit Image()
		: m_data(nullptr) {
	}

	Image(const Image& img)
		: m_data(img.m_data),
		m_texture(img.m_texture) {
	}

	Image& operator=(const Image& img) {
		m_data = img.m_data;
		m_texture = img.m_texture;
		return *this;
	}

	void clear() {
		m_data = nullptr;
		m_texture = nullptr;
	}

	const ivec2& size() const { return m_data->size; }
	const Data* data() const { return m_data; }
	const TexturePtr& texture() const { return m_texture; }
	int width() const { return m_data->size.x; }
	int height() const { return m_data->size.y; }

private:
	const Data* m_data;
	TexturePtr m_texture;
};

namespace ImageManager
{
	Image image(const string& filename);

	void loadProject(BinaryReader& reader);
}