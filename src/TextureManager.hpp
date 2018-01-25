#pragma once

#include "Texture.hpp"

namespace TextureManager
{
	TexturePtr getWaterTexture();

	TexturePtr getTerrainTexture(int id);

	TexturePtr getSkyTexture(int id);

	TexturePtr getCloudTexture(int id);

	TexturePtr getMoonTexture();

	TexturePtr getSunTexture();

	TexturePtr getModelTexture(const string& filename);

	TexturePtr getSfxTexture(const string& filename);

	TexturePtr getImageTexture(const string& filename);
}