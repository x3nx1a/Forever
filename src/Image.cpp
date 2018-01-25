#include "StdAfx.hpp"
#include "Image.hpp"
#include "TextureManager.hpp"

#include <unordered_map>

Image::Image(const Data* data)
	: m_data(data),
	m_texture(TextureManager::getImageTexture(*data->textureName))
{
}

namespace ImageManager
{
	namespace
	{
		vector<string> s_textureNames;

		unordered_map<string, Image::Data> s_images;
	}

	Image image(const string& filename)
	{
		auto it = s_images.find(filename);
		if (it != s_images.end())
			return Image(&it->second);
		else
		{
			emscripten_log(EM_LOG_ERROR, "Image not found '%s'", filename.c_str());
			return Image();
		}
	}

	void loadProject(BinaryReader& reader)
	{
		char buffer[128];
		Image::Data data;
		uint8_t len;

		const int textureCount = reader.read<int>();
		s_textureNames.resize(textureCount);
		for (int i = 0; i < textureCount; i++)
		{
			reader >> len;
			reader.read(buffer, len);
			buffer[len] = '\0';

			s_textureNames[i] = buffer;
		}

		const int spriteCount = reader.read<int>();
		for (int i = 0; i < spriteCount; i++)
		{
			data.textureName = &s_textureNames[reader.read<int>()];

			reader >> len;
			reader.read(buffer, len);
			buffer[len] = '\0';

			reader >> data.size.x
				>> data.size.y
				>> data.offset.x
				>> data.offset.y
				>> data.rotated
				>> data.coords.left
				>> data.coords.right
				>> data.coords.top
				>> data.coords.bottom;

			s_images.insert(pair<string, Image::Data>(buffer, data));
		}
	}
}