#include "StdAfx.hpp"
#include "TextureManager.hpp"
#include <unordered_map>

namespace TextureManager
{
	namespace
	{
		vector<pair<int, TexturePtr>> terrainTextures,
			skyTextures,
			cloudTextures;

		TexturePtr waterTexture = nullptr,
			m_moonTexture = nullptr,
			m_sunTexture = nullptr;

		unordered_map<string, TexturePtr> modelTextures, sfxTextures, imageTextures;
	}

	TexturePtr getWaterTexture()
	{
		if (!waterTexture)
		{
			waterTexture = TexturePtr::create("world/", "water", Texture::Compressed | Texture::QualityDependent);
			waterTexture->setMinFilter(GL_LINEAR_MIPMAP_NEAREST);
			waterTexture->setMagFilter(GL_LINEAR);
		}

		return waterTexture;
	}

	TexturePtr getMoonTexture()
	{
		if (!m_moonTexture)
		{
			m_moonTexture = TexturePtr::create("env/", "moon", Texture::Compressed);
			m_moonTexture->setMinFilter(GL_LINEAR);
			m_moonTexture->setMagFilter(GL_LINEAR);
		}

		return m_moonTexture;
	}

	TexturePtr getSunTexture()
	{
		if (!m_sunTexture)
		{
			m_sunTexture = TexturePtr::create("env/", "sun");
			m_sunTexture->setMinFilter(GL_LINEAR);
			m_sunTexture->setMagFilter(GL_LINEAR);
		}

		return m_sunTexture;
	}

	TexturePtr getTerrainTexture(int id)
	{
		for (std::size_t i = 0; i < terrainTextures.size(); i++)
			if (terrainTextures[i].first == id)
				return terrainTextures[i].second;

		TexturePtr newTexture = TexturePtr::create("world/", "terrain_" + to_string(id), Texture::Compressed | Texture::QualityDependent);

		newTexture->setMinFilter(GL_LINEAR_MIPMAP_NEAREST);
		newTexture->setMagFilter(GL_LINEAR);

		terrainTextures.push_back(pair<int, TexturePtr>(id, newTexture));
		return newTexture;
	}

	TexturePtr getSkyTexture(int id)
	{
		for (std::size_t i = 0; i < skyTextures.size(); i++)
			if (skyTextures[i].first == id)
				return skyTextures[i].second;

		TexturePtr newTexture = TexturePtr::create("env/", "skybox_" + to_string(id), Texture::Compressed | Texture::QualityDependent);

		newTexture->setMinFilter(GL_LINEAR);
		newTexture->setMagFilter(GL_LINEAR);

		skyTextures.push_back(pair<int, TexturePtr>(id, newTexture));
		return newTexture;
	}

	TexturePtr getCloudTexture(int id)
	{
		for (std::size_t i = 0; i < cloudTextures.size(); i++)
			if (cloudTextures[i].first == id)
				return cloudTextures[i].second;

		TexturePtr newTexture = TexturePtr::create("env/", "cloud_" + to_string(id), Texture::Compressed | Texture::QualityDependent);

		newTexture->setMinFilter(GL_LINEAR);
		newTexture->setMagFilter(GL_LINEAR);

		cloudTextures.push_back(pair<int, TexturePtr>(id, newTexture));
		return newTexture;
	}

	TexturePtr getModelTexture(const string& filename)
	{
		auto it = modelTextures.find(filename);
		if (it != modelTextures.end())
			return it->second;

		TexturePtr newTexture = TexturePtr::create("model/", filename, Texture::Compressed | Texture::QualityDependent);

		newTexture->setMinFilter(GL_LINEAR_MIPMAP_NEAREST);
		newTexture->setMagFilter(GL_LINEAR);

		modelTextures.insert(pair<string, TexturePtr>(filename, newTexture));

		return newTexture;
	}

	TexturePtr getSfxTexture(const string& filename)
	{
		auto it = sfxTextures.find(filename);
		if (it != sfxTextures.end())
			return it->second;

		TexturePtr newTexture = TexturePtr::create("model/", filename, Texture::Compressed);

		newTexture->setMinFilter(GL_LINEAR_MIPMAP_NEAREST);
		newTexture->setMagFilter(GL_LINEAR);

		sfxTextures.insert(pair<string, TexturePtr>(filename, newTexture));

		return newTexture;
	}

	TexturePtr getImageTexture(const string& filename)
	{
		auto it = imageTextures.find(filename);
		if (it != imageTextures.end())
			return it->second;

		TexturePtr newTexture = TexturePtr::create("ui/", filename);

		newTexture->setMinFilter(GL_LINEAR);
		newTexture->setMagFilter(GL_LINEAR);

		imageTextures.insert(pair<string, TexturePtr>(filename, newTexture));

		return newTexture;
	}
}