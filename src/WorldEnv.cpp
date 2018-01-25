#include "StdAfx.hpp"
#include "World.hpp"
#include "Shaders.hpp"
#include "GameTime.hpp"

namespace
{
	struct LightColor
	{
		float r1, g1, b1, r2, g2, b2;
	};

	const LightColor outdoorLightColors[] = {
		// diffuse, ambient
		0.4f,  0.4f,  0.5f,    0.3f, 0.3f, 0.4f,  // 0 12
		0.4f,  0.4f,  0.5f,    0.3f, 0.3f, 0.4f,  // 1
		0.4f,  0.4f,  0.5f,    0.3f, 0.3f, 0.4f,  // 2
		0.4f,  0.4f,  0.5f,    0.3f, 0.3f, 0.4f,  // 3
		0.4f,  0.4f,  0.5f,    0.3f, 0.3f, 0.4f,  // 4
		0.4f,  0.4f,  0.5f,    0.3f, 0.3f, 0.4f,  // 5
		0.5f,  0.5f,  0.6f,    0.4f, 0.4f, 0.4f, // 6
		0.7f,  0.7f,  0.7f,    0.5f, 0.5f, 0.5f, // 7
		0.8f,  0.8f,  0.8f,    0.5f, 0.5f, 0.5f, // 8
		0.9f,  0.9f,  0.9f,    0.5f, 0.5f, 0.5f, // 9
		1.0f,  1.0f,  1.0f,    0.5f, 0.5f, 0.5f, // 10
		1.0f,  1.0f,  1.0f,    0.6f, 0.6f, 0.6f, // 11
		1.0f,  1.0f,  1.0f,    0.6f, 0.6f, 0.6f, // 12
		1.0f,  1.0f,  1.0f,    0.6f, 0.6f, 0.6f, // 13 1
		1.0f,  1.0f,  1.0f,    0.6f, 0.6f, 0.6f, // 14 2
		1.0f,  1.0f,  1.0f,    0.5f, 0.5f, 0.5f, // 15 3
		0.9f,  0.9f,  0.9f,    0.5f, 0.5f, 0.5f, // 16 5
		0.9f,  0.6f,  0.2f,    0.5f, 0.5f, 0.4f, // 17 6
		0.6f,  0.6f,  0.4f,    0.4f, 0.4f, 0.4f, // 18 7
		0.5f,  0.5f,  0.4f,    0.4f, 0.4f, 0.4f, // 19 8
		0.45f, 0.45f, 0.4f,    0.35f, 0.35f, 0.35f, // 20 8
		0.43f, 0.43f, 0.5f,    0.33f, 0.33f, 0.3f, // 21 9
		0.41f, 0.41f, 0.5f,    0.31f, 0.31f, 0.3f, // 22 10
		0.4f,  0.4f,  0.5f,    0.3f, 0.3f, 0.4f  // 23 11
	};
}

void World::setLight()
{
	vec3 ambient, diffuse, fogDiffuse, lightDir;

	if (m_inDoor)
	{
		fogDiffuse = m_diffuse;

		diffuse = fogDiffuse + vec3(0.1f);
		ambient = m_ambient * 0.9f;

		lightDir = m_lightDir;
	}
	else
	{
		int hour = GameTime::hour();
		const int min = GameTime::min();
		const int sec = GameTime::sec();

		hour--;
		if (hour < 0)
			hour = 0;
		if (hour > 23)
			hour = 23;

		const LightColor lightColor = outdoorLightColors[hour];

		LightColor lightColorPrv = outdoorLightColors[(hour - 1 == -1) ? 23 : hour - 1];
		lightColorPrv.r1 += (lightColor.r1 - lightColorPrv.r1) * min / 60;
		lightColorPrv.g1 += (lightColor.g1 - lightColorPrv.g1) * min / 60;
		lightColorPrv.b1 += (lightColor.b1 - lightColorPrv.b1) * min / 60;
		lightColorPrv.r2 += (lightColor.r2 - lightColorPrv.r2) * min / 60;
		lightColorPrv.g2 += (lightColor.g2 - lightColorPrv.g2) * min / 60;
		lightColorPrv.b2 += (lightColor.b2 - lightColorPrv.b2) * min / 60;

		fogDiffuse.r = lightColorPrv.r1;
		fogDiffuse.g = lightColorPrv.g1;
		fogDiffuse.b = lightColorPrv.b1;

		ambient.r = lightColorPrv.r2;
		ambient.g = lightColorPrv.g2;
		ambient.b = lightColorPrv.b2;

		diffuse = fogDiffuse * 1.1f;
		ambient *= 0.9f;

		float sunAngle;
		hour = GameTime::hour();

		if (hour >= 20 || hour <= 6)
		{
			int curHour;
			if (hour >= 20)
				curHour = hour - 20;
			else
				curHour = hour + 4;

			sunAngle = 180.0f - (float)(curHour * 3600 + min * 60 + sec) / (float)(11 * 3600) * 180.0f;
		}
		else if (hour >= 7 && hour <= 17)
			sunAngle = (float)((hour - 7) * 3600 + min * 60 + sec) / (float)(11 * 3600) * 180.0f;
		else
			sunAngle = 180.0f;

		sunAngle = radians(sunAngle);
		lightDir = vec3(-cos(sunAngle), -sin(sunAngle), 0);

		ShaderVars::sunAngle = sunAngle;
	}

	ShaderVars::lightDir = normalize(lightDir);
	ShaderVars::fogColor = fogDiffuse;

	ambient *= 2.0f;

	const vec3 terrainLightDir = -normalize(lightDir);

	Shaders::terrain.use();
	gl::uniform(Shaders::terrain.uAmbient, ambient);
	gl::uniform(Shaders::terrain.uDiffuse, diffuse);
	gl::uniform(Shaders::terrain.uLightDir, terrainLightDir);
	gl::uniform(Shaders::terrain.uFogColor, fogDiffuse);

	Shaders::object.use();
	gl::uniform(Shaders::object.uAmbient, ambient);
	gl::uniform(Shaders::object.uDiffuse, diffuse);
	gl::uniform(Shaders::object.uLightDir, terrainLightDir);
	gl::uniform(Shaders::object.uFogColor, fogDiffuse);

	Shaders::skin.use();
	gl::uniform(Shaders::skin.uAmbient, ambient);
	gl::uniform(Shaders::skin.uDiffuse, diffuse);
	gl::uniform(Shaders::skin.uLightDir, terrainLightDir);
	gl::uniform(Shaders::skin.uFogColor, fogDiffuse);

	Shaders::water.use();
	gl::uniform(Shaders::water.uFogColor, fogDiffuse);

	Shaders::cloud.use();
	gl::uniform(Shaders::cloud.uFogColor, fogDiffuse);
}