#include "StdAfx.hpp"
#include "Skybox.hpp"
#include "Shaders.hpp"
#include "GameTime.hpp"
#include "GeometryUtils.hpp"
#include "TextureManager.hpp"
#include "World.hpp"
#include "Config.hpp"
#include "Sfx.hpp"

#include "../gameres/defineObj.h"

#define SKY_VERTEX_COUNT (12 * 28)

Skybox::Skybox(World* world)
	: m_world(world),
	m_cloudPos(0.0f),
	m_weather(WEATHER_NONE),
	m_falls(nullptr),
	m_fallCount(0),
	m_rainVertices(nullptr),
	m_snowVertices(nullptr)
{
	m_skyTextures[0] = TextureManager::getSkyTexture(1);
	m_skyTextures[1] = TextureManager::getSkyTexture(2);
	m_skyTextures[2] = TextureManager::getSkyTexture(3);

	m_cloudTextures[0] = TextureManager::getCloudTexture(1);
	m_cloudTextures[1] = TextureManager::getCloudTexture(2);
	m_cloudTextures[2] = TextureManager::getCloudTexture(3);
}

Skybox::~Skybox()
{
	if (m_falls)
		delete[] m_falls;
	if (m_rainVertices)
		delete[] m_rainVertices;
	if (m_snowVertices)
		delete[] m_snowVertices;
}

void Skybox::update(int frameCount)
{
	m_cloudPos = mod(m_cloudPos + 0.0002f * frameCount, 1.0f);

	const Weather weather = m_world->weather();
	if (weather != m_weather)
	{
		m_weather = weather;
		initWeather();
	}

	if (Config::weatherEffects && (m_weather == WEATHER_RAIN || m_weather == WEATHER_SNOW))
	{
		for (int i = 0; i < m_fallCount; i++)
		{
			Fall& fall = m_falls[i];

			if (m_world->getLandHeight_fast(fall.pos.x, fall.pos.z) - 1.0f > fall.pos.y)
			{
				if (m_weather == WEATHER_RAIN && ((rand() % 10) >= 7))
				{
					const WaterHeight* const waterHeight = m_world->getWaterHeight(fall.pos);
					const float landHeight = m_world->getLandHeight(fall.pos.x, fall.pos.z);

					vec3 pos(fall.pos.x, 0.0f, fall.pos.z);
					float scale;
					vec3 rot;

					if (waterHeight && waterHeight->type == WaterHeight::Water && waterHeight->height >= landHeight)
					{
						pos.y = waterHeight->height;
						scale = ((float)(rand() % 2) + 1.0f) * 0.1f;
					}
					else
					{
						pos.y = landHeight + 0.1f;
						scale = ((float)(rand() % 2) + 0.05f) * 0.1f;
						rot = degrees(m_world->getLandRot(pos.x, pos.z));
					}

					Sfx* rainCircle = Sfx::create(m_world, XI_GEN_RAINCIRCLE01, pos);
					rainCircle->setScale(vec3(scale));
					rainCircle->setRot(rot);
				}

				fall.pos = vec3(
					ShaderVars::playerPos.x + (float(rand() - RAND_MAX / 2) / RAND_MAX * 25),
					ShaderVars::playerPos.y + 10 + (float(rand()) / RAND_MAX * 10),
					ShaderVars::playerPos.z + (float(rand() - RAND_MAX / 2) / RAND_MAX * 25)
				);
			}
			else
				fall.pos += fall.velocity * (float)frameCount;
		}
	}
}

void Skybox::updateView()
{
	const vec3 eye(0.0f, ShaderVars::cameraPos.y / 200.0f - 0.4f, 0.0f);
	const vec3 center = eye + ShaderVars::cameraDir;

	const mat4 view = lookAt(eye, center, vec3(0, 1, 0));
	const mat4 WVP = ShaderVars::proj * view;

	Shaders::skybox.use();
	gl::uniform(Shaders::skybox.uWVP, WVP);

	if (Config::weatherEffects)
	{
		if (m_weather == WEATHER_RAIN)
		{
			Shaders::rain.use();
			gl::uniform(Shaders::rain.uWVP, ShaderVars::viewProj);
		}
		else if (m_weather == WEATHER_SNOW)
		{
			Shaders::snow.use();
			gl::uniform(Shaders::snow.uWVP, ShaderVars::viewProj);
			gl::uniform(Shaders::snow.uCameraPos, ShaderVars::cameraPos);
			gl::uniform(Shaders::snow.uSizeFactor, ShaderVars::pixelRatio);
		}
	}
}

void Skybox::initWeather()
{
	if (m_falls)
	{
		delete[] m_falls;
		m_falls = nullptr;
	}
	if (m_rainVertices)
	{
		delete[] m_rainVertices;
		m_rainVertices = nullptr;
	}
	if (m_snowVertices)
	{
		delete[] m_snowVertices;
		m_snowVertices = nullptr;
	}

	m_fallCount = 0;

	if (m_weather == WEATHER_RAIN || m_weather == WEATHER_SNOW)
	{
		m_fallCount = 300;
		m_falls = new Fall[m_fallCount];

		if (m_weather == WEATHER_RAIN)
		{
			m_rainVertices = new RainVertex[m_fallCount * 2];

			Shaders::rain.use();
			gl::uniform(Shaders::rain.uWVP, ShaderVars::viewProj);
		}
		else if (m_weather == WEATHER_SNOW)
		{
			m_snowVertices = new SnowVertex[m_fallCount];

			Shaders::snow.use();
			gl::uniform(Shaders::snow.uWVP, ShaderVars::viewProj);
		}

		for (int i = 0; i < m_fallCount; i++)
		{
			Fall& fall = m_falls[i];

			fall.pos = vec3(
				ShaderVars::playerPos.x + (float(rand() - RAND_MAX / 2) / RAND_MAX * 25),
				ShaderVars::playerPos.y + 10 + (float(rand()) / RAND_MAX * 10),
				ShaderVars::playerPos.z + (float(rand() - RAND_MAX / 2) / RAND_MAX * 25)
			);

			if (m_weather == WEATHER_RAIN)
			{
				m_rainVertices[i * 2].a = 0.39f;
				m_rainVertices[i * 2 + 1].a = 0.0f;

				fall.velocity = vec3(0.0f, float((-1) * (int)(rand() % 10) + (-7)) / 200 - 0.05f, 0.0f) * 1.5f;
			}
			else
				fall.velocity = vec3((float)((rand() % 50) - 25) / 800, ((float)((-1) * (int)(rand() % 100)) / 200 - 0.05f) / 4.0f, (float)((rand() % 50) - 25) / 800);
		}
	}
}

void Skybox::renderWeather()
{
	if (!Config::weatherEffects)
		return;

	gl::enableBlend();
	gl::disableCull();
	gl::disableDepthWrite();
	gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (m_weather == WEATHER_RAIN)
	{
		for (int i = 0; i < m_fallCount; i++)
		{
			m_rainVertices[i * 2].p = m_falls[i].pos;
			m_rainVertices[i * 2 + 1].p = m_falls[i].pos + vec3(0.0f, 2.0f, 0.0f);
		}

		Shaders::rain.use();

		ShaderVars::rainVAO.bind();
		ShaderVars::rainVBO.data(m_fallCount * 2 * sizeof(RainVertex), m_rainVertices, true);

		gl::lineWidth(ceil(ShaderVars::pixelRatio));

		gl::drawArrays(GL_LINES, 0, m_fallCount * 2);
	}
	else if (m_weather == WEATHER_SNOW)
	{
		for (int i = 0; i < m_fallCount; i++)
			m_snowVertices[i].p = m_falls[i].pos;

		Shaders::snow.use();

		ShaderVars::snowVAO.bind();
		ShaderVars::snowVBO.data(m_fallCount * sizeof(SnowVertex), m_snowVertices, true);

		gl::drawArrays(GL_POINTS, 0, m_fallCount);
	}
}

void Skybox::render()
{
	gl::disableDepthTest();
	gl::disableDepthWrite();
	gl::disableCull();
	gl::enableBlend();
	gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shaders::skybox.use();
	ShaderVars::skyboxVAO.bind();

	gl::uniform(Shaders::skybox.uTextureOffset, 0.0f);

	const int hour = GameTime::hour();
	const int min = GameTime::min();
	const float alphaEx = 1.0f;

	if (hour < 6)
	{
		gl::uniform(Shaders::skybox.uAlphaFactor, alphaEx);
		m_skyTextures[2]->bind();
		gl::drawArrays(GL_TRIANGLES, 0, SKY_VERTEX_COUNT);
	}
	else if (hour == 6)
	{
		const float factor = ((float)GameTime::min()) / 59.0f;

		gl::uniform(Shaders::skybox.uAlphaFactor, (1.0f - factor) * alphaEx);
		m_skyTextures[2]->bind();
		gl::drawArrays(GL_TRIANGLES, 0, SKY_VERTEX_COUNT);

		gl::uniform(Shaders::skybox.uAlphaFactor, factor * alphaEx);
		m_skyTextures[0]->bind();
		gl::drawArrays(GL_TRIANGLES, 0, SKY_VERTEX_COUNT);
	}
	else if (hour < 17)
	{
		gl::uniform(Shaders::skybox.uAlphaFactor, alphaEx);
		m_skyTextures[0]->bind();
		gl::drawArrays(GL_TRIANGLES, 0, SKY_VERTEX_COUNT);
	}
	else if (hour == 17 || hour == 18)
	{
		const float factor = ((float)(hour == 17 ? GameTime::min() : GameTime::min() + 60)) / 119.0f;

		gl::uniform(Shaders::skybox.uAlphaFactor, (1.0f - factor) * alphaEx);
		m_skyTextures[0]->bind();
		gl::drawArrays(GL_TRIANGLES, 0, SKY_VERTEX_COUNT);

		gl::uniform(Shaders::skybox.uAlphaFactor, factor * alphaEx);
		m_skyTextures[1]->bind();
		gl::drawArrays(GL_TRIANGLES, 0, SKY_VERTEX_COUNT);
	}
	else if (hour == 19)
	{
		const float factor = ((float)GameTime::min()) / 59.0f;

		gl::uniform(Shaders::skybox.uAlphaFactor, (1.0f - factor) * alphaEx);
		m_skyTextures[1]->bind();
		gl::drawArrays(GL_TRIANGLES, 0, SKY_VERTEX_COUNT);

		gl::uniform(Shaders::skybox.uAlphaFactor, factor * alphaEx);
		m_skyTextures[2]->bind();
		gl::drawArrays(GL_TRIANGLES, 0, SKY_VERTEX_COUNT);
	}
	else
	{
		gl::uniform(Shaders::skybox.uAlphaFactor, alphaEx);
		m_skyTextures[2]->bind();
		gl::drawArrays(GL_TRIANGLES, 0, SKY_VERTEX_COUNT);
	}

	gl::uniform(Shaders::skybox.uTextureOffset, m_cloudPos);

	if (hour < 6)
	{
		gl::uniform(Shaders::skybox.uAlphaFactor, alphaEx);
		m_cloudTextures[2]->bind();
		gl::drawArrays(GL_TRIANGLE_FAN, SKY_VERTEX_COUNT, 30);
	}
	else if (hour == 6)
	{
		const float factor = ((float)GameTime::min()) / 59.0f;

		gl::uniform(Shaders::skybox.uAlphaFactor, (1.0f - factor) * alphaEx);
		m_cloudTextures[2]->bind();
		gl::drawArrays(GL_TRIANGLE_FAN, SKY_VERTEX_COUNT, 30);

		gl::uniform(Shaders::skybox.uAlphaFactor, factor * alphaEx);
		m_cloudTextures[0]->bind();
		gl::drawArrays(GL_TRIANGLE_FAN, SKY_VERTEX_COUNT, 30);
	}
	else if (hour < 17)
	{
		gl::uniform(Shaders::skybox.uAlphaFactor, alphaEx);
		m_cloudTextures[0]->bind();
		gl::drawArrays(GL_TRIANGLE_FAN, SKY_VERTEX_COUNT, 30);
	}
	else if (hour == 17 || hour == 18)
	{
		const float factor = ((float)(hour == 17 ? GameTime::min() : GameTime::min() + 60)) / 119.0f;

		gl::uniform(Shaders::skybox.uAlphaFactor, (1.0f - factor) * alphaEx);
		m_cloudTextures[0]->bind();
		gl::drawArrays(GL_TRIANGLE_FAN, SKY_VERTEX_COUNT, 30);

		gl::uniform(Shaders::skybox.uAlphaFactor, factor * alphaEx);
		m_cloudTextures[1]->bind();
		gl::drawArrays(GL_TRIANGLE_FAN, SKY_VERTEX_COUNT, 30);
	}
	else if (hour == 19)
	{
		const float factor = ((float)GameTime::min()) / 59.0f;

		gl::uniform(Shaders::skybox.uAlphaFactor, (1.0f - factor) * alphaEx);
		m_cloudTextures[1]->bind();
		gl::drawArrays(GL_TRIANGLE_FAN, SKY_VERTEX_COUNT, 30);

		gl::uniform(Shaders::skybox.uAlphaFactor, factor * alphaEx);
		m_cloudTextures[2]->bind();
		gl::drawArrays(GL_TRIANGLE_FAN, SKY_VERTEX_COUNT, 30);
	}
	else
	{
		gl::uniform(Shaders::skybox.uAlphaFactor, alphaEx);
		m_cloudTextures[2]->bind();
		gl::drawArrays(GL_TRIANGLE_FAN, SKY_VERTEX_COUNT, 30);
	}

	if (hour <= 17 || hour >= 20)
	{
		const bool isSun = hour >= 7 && hour <= 17;
		const float sunSize = isSun ? 0.5f : 0.2f;

		const mat3 tm = mat3(rotate(mat4(), pi<float>() * 2.0f - ShaderVars::sunAngle, vec3(0, 0, 1)));

		static SkyboxVertex vertices[4];
		vertices[0].p = vec3(3.0f, -sunSize, sunSize) * tm;
		vertices[0].tu1 = 0.0f;
		vertices[0].tv1 = 0.0f;
		vertices[0].a = 1.0f;

		vertices[1].p = vec3(3.0f, sunSize, sunSize) * tm;
		vertices[1].tu1 = 1.0f;
		vertices[1].tv1 = 0.0f;
		vertices[1].a = 1.0f;

		vertices[2].p = vec3(3.0f, sunSize, -sunSize) * tm;
		vertices[2].tu1 = 1.0f;
		vertices[2].tv1 = 1.0f;
		vertices[2].a = 1.0f;

		vertices[3].p = vec3(3.0f, -sunSize, -sunSize) * tm;
		vertices[3].tu1 = 0.0f;
		vertices[3].tv1 = 1.0f;
		vertices[3].a = 1.0f;

		gl::uniform(Shaders::skybox.uTextureOffset, 0.0f);
		gl::uniform(Shaders::skybox.uAlphaFactor, 1.0f);

		if (isSun)
			TextureManager::getSunTexture()->bind();
		else
		{
			gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
			TextureManager::getMoonTexture()->bind();
		}

		ShaderVars::sunVAO.bind();

		ShaderVars::sunVBO.bind();
		ShaderVars::sunVBO.data(sizeof(vertices), &vertices, true);

		gl::drawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
}