#pragma once

#include "Vertex.hpp"
#include "ShaderVars.hpp"

namespace Shaders
{
	class TerrainProgram : public gl::Program
	{
	public:
		int uWVP, uLightMapOffset, uCameraPos, uFogSettings, uFogColor, uAmbient, uDiffuse, uLightDir;
	};

	class WaterProgram : public gl::Program
	{
	public:
		int uWVP, uTextureOffset, uCameraPos, uFogSettings, uFogColor;
	};

	class CloudProgram : public gl::Program
	{
	public:
		int uWVP, uTextureOffset, uCameraPos, uFogSettings, uFogColor, uHeightOffset;
	};

	class SkyboxProgram : public gl::Program
	{
	public:
		int uWVP, uTextureOffset, uAlphaFactor;
	};

	class ObjectProgram : public gl::Program
	{
	public:
		int uWVP, uCameraPos, uFogSettings, uFogColor, uWorld, uAmbient, uDiffuse, uLightDir, uEffects;
		ivec3 curEffects;
	};

	class SkinProgram : public ObjectProgram
	{
	public:
		int uBones;
	};

	class SfxProgram : public gl::Program
	{
	public:
		int uWVP, uAlphaFactor;
		float alphaFactor;
	};

	class RainProgram : public gl::Program
	{
	public:
		int uWVP;
	};

	class SnowProgram : public gl::Program
	{
	public:
		int uWVP, uSizeFactor, uCameraPos;
	};

	class Render2DProgram : public gl::Program
	{
	public:
		int uWVP;
	};

	extern TerrainProgram terrain;
	extern WaterProgram water;
	extern CloudProgram cloud;
	extern SkyboxProgram skybox;
	extern ObjectProgram object;
	extern SkinProgram skin;
	extern SfxProgram sfx;
	extern RainProgram rain;
	extern SnowProgram snow;
	extern Render2DProgram render2d;

	void createAll();
}