#include "StdAfx.hpp"
#include "World.hpp"
#include "Shaders.hpp"
#include "GeometryUtils.hpp"
#include "Skybox.hpp"
#include "TextureManager.hpp"
#include "Object.hpp"
#include "Canvas2D.hpp"
#include "Config.hpp"

namespace
{
	const vec2 waterTextureOffsets[] = {
		vec2(0.0f, 0.0f), vec2(0.25f, 0.0f), vec2(0.5f, 0.0f), vec2(0.75f, 0.0f), vec2(0.0f, 0.25f), vec2(0.25f, 0.25f), vec2(0.5f, 0.25f), vec2(0.75f, 0.25f), vec2(0.0f, 0.5f), vec2(0.25f, 0.5f), vec2(0.5f, 0.5f), vec2(0.75f, 0.5f), vec2(0.0f, 0.75f), vec2(0.25f, 0.75f), vec2(0.5f, 0.75f), vec2(0.75f, 0.75f)
	};

	const u8vec4 waterTextureColors[] = {
		u8vec4(24, 101, 133, 127),
		u8vec4(8, 49, 32, 127)
	};
}

void World::cullObjects()
{
	m_cullObjCount = 0;
	m_cullSfxCount = 0;

	int i, type;
	std::size_t j;

	for (i = 0; i < m_cullLandCount; i++)
	{
		for (type = 0; type < MAX_OBJTYPE; type++)
		{
			const vector<Object*>& objects = m_cullLands[i]->objects((ObjectType)type);
			for (j = 0; j < objects.size(); j++)
			{
				Object* const obj = objects[j];
				if (!isValidObject(obj))
					continue;

				obj->cull();

				if (obj->visible())
				{
					if (obj->model()->modelType() != MODELTYPE_SFX)
					{
						m_cullObj[m_cullObjCount] = obj;
						m_cullObjCount++;
					}
					else
					{
						m_cullSfx[m_cullSfxCount] = obj;
						m_cullSfxCount++;
					}
				}
			}
		}
	}

	if (m_cullObjCount > 0)
		sort(m_cullObj, m_cullObj + m_cullObjCount, Object::sortFarToNear);
	if (m_cullSfxCount > 0)
		sort(m_cullSfx, m_cullSfx + m_cullSfxCount, Object::sortFarToNear);
}

void World::renderTerrain()
{
	gl::enableCull();
	gl::enableDepthWrite();
	gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shaders::terrain.use();

	for (int i = 0; i < m_cullLandCount; i++)
		m_cullLands[i]->render();
}

void World::renderWater()
{
	gl::disableCull();
	gl::disableDepthWrite();
	gl::enableBlend();
	gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shaders::water.use();
	TextureManager::getWaterTexture()->bind();

	const vec2 offset = waterTextureOffsets[(int)m_waterFrame];
	gl::uniform(Shaders::water.uTextureOffset, offset);

	for (int i = 0; i < m_cullLandCount; i++)
		m_cullLands[i]->renderWater(WaterHeight::Water);

	Shaders::cloud.use();
	m_cloudTexture->bind();

	gl::uniform(Shaders::cloud.uHeightOffset, -40.0f);
	gl::uniform(Shaders::cloud.uTextureOffset, m_cloudsPos[0]);

	for (int i = 0; i < m_cullLandCount; i++)
		m_cullLands[i]->renderWater(WaterHeight::Cloud);

	gl::uniform(Shaders::cloud.uHeightOffset, 0.0f);
	gl::uniform(Shaders::cloud.uTextureOffset, m_cloudsPos[1]);

	for (int i = 0; i < m_cullLandCount; i++)
		m_cullLands[i]->renderWater(WaterHeight::Cloud);
}

void World::render()
{
	if (!loaded()) return;

	const ivec2 pos = posToLand(ShaderVars::cameraPos);
	ivec2 p;

	m_cullLandCount = 0;
	for (p.y = pos.y - m_visibilityLand; p.y <= pos.y + m_visibilityLand; p.y++)
	{
		for (p.x = pos.x - m_visibilityLand; p.x <= pos.x + m_visibilityLand; p.x++)
		{
			const int offset = p.y * m_size.x + p.x;
			if (landInWorld(p) && m_lands[offset] && m_lands[offset]->visible() && m_cullLandCount < MAX_CULL_LANDS)
			{
				m_cullLands[m_cullLandCount] = m_lands[offset].get();
				m_cullLandCount++;
			}
		}
	}

	setLight();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	gl::enableDepthWrite();
	glClearColor(ShaderVars::fogColor.r, ShaderVars::fogColor.g, ShaderVars::fogColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

	if (m_skybox)
		m_skybox->render();

	gl::enableDepthTest();

	renderTerrain();

	for (int i = 0; i < m_cullObjCount; i++)
		m_cullObj[i]->render();

	renderWater();

	for (int i = 0; i < m_cullSfxCount; i++)
		m_cullSfx[i]->render();

	if (m_weather != WEATHER_NONE && m_skybox)
		m_skybox->renderWeather();

	Canvas2D::setViewRect(ShaderVars::viewport);
	Canvas2D::globalAlpha = 1.0f;

	const WaterHeight* waterHeight = getWaterHeight(ShaderVars::cameraPos);
	if (waterHeight != nullptr && waterHeight->type == WaterHeight::Water && waterHeight->height >= ShaderVars::cameraPos.y)
	{
		Canvas2D::fillStyle = waterTextureColors[waterHeight->texture];
		Canvas2D::fillRect(ShaderVars::viewport);
	}

	Canvas2D::fillStyle = u8vec4(255);
}

void World::updateView()
{
	const ivec2 pos = posToLand(m_cameraPos);
	const float farPlaneFactor = glm::min(0.25f + Config::fieldViewFactor / 1.33f, 1.0f);

	m_farPlane = 512.0f * farPlaneFactor;
	m_visibilityLand = glm::max((int)(m_farPlane / (MAP_SIZE * m_MPU)), 1);

	ShaderVars::cameraPos = m_cameraPos;
	ShaderVars::cameraDir = normalize(m_cameraTarget - m_cameraPos);

	ShaderVars::playerPos = m_cameraPos;

	ShaderVars::view = lookAt(m_cameraPos, m_cameraTarget, vec3(0, 1, 0));

	ShaderVars::invView = inverse(ShaderVars::view);
	ShaderVars::invView[3][0] = ShaderVars::invView[3][1] = ShaderVars::invView[3][2] = 0.0f;

	ShaderVars::proj = perspective(pi<float>() / 4.0f, (float)ShaderVars::viewport.width() / (float)ShaderVars::viewport.height(), 0.5f, m_farPlane);

	ShaderVars::MPU = m_MPU;
	ShaderVars::viewProj = ShaderVars::proj * ShaderVars::view;

	const mat4 invWVP = inverse(ShaderVars::viewProj);

	vec3 frustum[] = {
		vec3(-1.0f, -1.0f, 0.0f), // xyz
		vec3(1.0f, -1.0f, 0.0f), // Xyz
		vec3(-1.0f, 1.0f, 0.0f), // xYz
		vec3(1.0f, 1.0f, 0.0f), // XYz
		vec3(-1.0f, -1.0f, 1.0f), // xyZ
		vec3(1.0f, -1.0f, 1.0f), // XyZ
		vec3(-1.0f, 1.0f, 1.0f), // xYZ
		vec3(1.0f, 1.0f, 1.0f) // XYZ
	};

	for (int i = 0; i < 8; i++)
		frustum[i] = transformCoord(frustum[i], invWVP);

	ShaderVars::frustum[0] = planeFromPoints(frustum[0], frustum[1], frustum[2]); // Near
	ShaderVars::frustum[1] = planeFromPoints(frustum[6], frustum[7], frustum[5]); // Far
	ShaderVars::frustum[2] = planeFromPoints(frustum[2], frustum[6], frustum[4]); // Left
	ShaderVars::frustum[3] = planeFromPoints(frustum[7], frustum[3], frustum[5]); // Right
	ShaderVars::frustum[4] = planeFromPoints(frustum[2], frustum[3], frustum[6]); // Top
	ShaderVars::frustum[5] = planeFromPoints(frustum[1], frustum[0], frustum[4]); // Bottom

	const float fogStart = m_fogStart * farPlaneFactor;
	const float fogEnd = m_fogEnd * farPlaneFactor;
	const vec2 fogSettings = vec2(fogEnd, 1.0f / (fogEnd - fogStart));

	Shaders::terrain.use();
	gl::uniform(Shaders::terrain.uWVP, ShaderVars::viewProj);
	gl::uniform(Shaders::terrain.uCameraPos, ShaderVars::cameraPos);
	gl::uniform(Shaders::terrain.uFogSettings, fogSettings);

	Shaders::water.use();
	gl::uniform(Shaders::water.uWVP, ShaderVars::viewProj);
	gl::uniform(Shaders::water.uCameraPos, ShaderVars::cameraPos);
	gl::uniform(Shaders::water.uFogSettings, fogSettings);

	Shaders::cloud.use();
	gl::uniform(Shaders::cloud.uWVP, ShaderVars::viewProj);
	gl::uniform(Shaders::cloud.uCameraPos, ShaderVars::cameraPos);
	gl::uniform(Shaders::cloud.uFogSettings, fogSettings);

	Shaders::object.use();
	gl::uniform(Shaders::object.uCameraPos, ShaderVars::cameraPos);
	gl::uniform(Shaders::object.uFogSettings, fogSettings);

	Shaders::skin.use();
	gl::uniform(Shaders::skin.uCameraPos, ShaderVars::cameraPos);
	gl::uniform(Shaders::skin.uFogSettings, fogSettings);

	if (m_skybox)
		m_skybox->updateView();

	ivec2 p;
	LandscapePtr land;

	for (p.y = pos.y - m_visibilityLand; p.y <= pos.y + m_visibilityLand; p.y++)
	{
		for (p.x = pos.x - m_visibilityLand; p.x <= pos.x + m_visibilityLand; p.x++)
		{
			if (landInWorld(p))
			{
				land = m_lands[p.y * m_size.x + p.x];

				if (!land)
				{
					char buffer[256];
					sprintf(buffer, "world/%s/p%02d-%02d.bin", m_name.c_str(), p.x, p.y);

					land = m_lands[p.y * m_size.x + p.x] = LandscapePtr::create(buffer, this, p);
				}
				else if (land->loaded())
					land->updateCull();
			}
		}
	}
}

void World::setCameraPos(const vec3& pos)
{
	if (m_cameraPos != pos)
	{
		m_cameraPos = pos;
		m_updateView = true;
	}
}

void World::setCameraTarget(const vec3& target)
{
	if (m_cameraTarget != target)
	{
		m_cameraTarget = target;
		m_updateView = true;
	}
}

void World::setUpdateView()
{
	m_updateView = true;
}