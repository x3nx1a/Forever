#include "StdAfx.hpp"
#include "Landscape.hpp"
#include "World.hpp"
#include "Shaders.hpp"
#include "TextureManager.hpp"

namespace
{
	const u8vec4 waterTextureColors[] = {
		u8vec4(255, 255, 255, 255),
		u8vec4(84, 120, 60, 255)
	};
}

Landscape::Landscape(const string& filename, World* world, const ivec2& pos)
	: Resource(filename),
	m_world(world),
	m_pos(pos),
	m_visible(false),
	m_layerCount(0),
	m_layers(nullptr),
	m_waterVertexCount(0),
	m_cloudVertexCount(0),
	m_waterVertexOffset(0),
	m_cloudVertexOffset(0),
	m_lightMapData(nullptr)
{
	startLoad();
}

Landscape::~Landscape()
{
	if (gl::isContextActive())
		onContextLost();

	for (int i = 0; i < MAX_OBJTYPE; i++)
	{
		vector<Object*>& objs = m_objects[i];
		for (std::size_t j = 0; j < objs.size(); j++)
			delete objs[j];
		objs.clear();
	}

	if (m_layers)
		delete[] m_layers;

	if (m_lightMapData)
		delete[] m_lightMapData;
}

void Landscape::addObjArray(Object* obj)
{
	ObjectType type = obj->type();
	if (type == OT_OBJ && obj->model()->isAnimated())
		type = OT_ANI;
	m_objects[type].push_back(obj);
}

void Landscape::removeObjArray(Object* obj)
{
	vector<Object*>& arr = m_objects[obj->type()];
	auto it = find(arr.begin(), arr.end(), obj);
	if (it != arr.end())
		arr.erase(it);
}

bool Landscape::insertObjLink(Object* obj)
{
	return true;
}

void Landscape::render()
{
	m_VAO.bind();
	m_lightMap.bind(1);

	static bool patchRendered[NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE];
	memset(patchRendered, 0, sizeof(patchRendered));

	int i, j;
	bool layerBind;

	for (i = 0; i < m_layerCount; i++)
	{
		const Layer& layer = m_layers[i];

		layerBind = false;

		for (j = 0; j < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; j++)
		{
			if (layer.patchEnabled[j])
			{
				const Patch& patch = m_patches[j];

				if (patch.visible)
				{
					if (!layerBind)
					{
						gl::uniform(Shaders::terrain.uLightMapOffset, layer.lightMapOffset);
						layer.texture->bind();
						layerBind = true;
					}

					if (patchRendered[j])
						gl::enableBlend();
					else
					{
						gl::disableBlend();
						patchRendered[j] = true;
					}

					gl::drawElements<uint16_t>(GL_TRIANGLES, 128 * 3, patch.indexOffset);
				}
			}
		}
	}
}

void Landscape::renderWater(WaterHeight::Type type)
{
	if (type == WaterHeight::Water)
	{
		if (m_waterVertexCount)
		{
			m_waterVAO.bind();
			gl::drawElements<uint16_t>(GL_TRIANGLES, (m_waterVertexCount * 6) / 4, 0);
		}
	}
	else
	{
		if (m_cloudVertexCount)
		{
			m_cloudVAO.bind();
			gl::drawElements<uint16_t>(GL_TRIANGLES, (m_cloudVertexCount * 6) / 4, 0);
		}
	}
}

void Landscape::onContextLost()
{
	m_VBO.destroy();
	m_VAO.destroy();
	m_cloudVAO.destroy();
	m_waterVAO.destroy();
	m_lightMap.destroy();
}

void Landscape::onContextRestored()
{
	if (!m_lightMapData)
		return;

	m_lightMap.create();
	m_lightMap.bind();
	m_lightMap.image(0, GL_RGBA, m_lightMapSize.x, m_lightMapSize.y, m_lightMapData);
	m_lightMap.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_lightMap.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setVertices();
}

void Landscape::onLoad(BinaryReader reader)
{
	const uint8_t ver = reader.read<uint8_t>();

	ivec2 p;
	reader >> p.x
		>> p.y;

	if (p != m_pos)
	{
		emscripten_log(EM_LOG_ERROR, "Lanscape %02d %02d '%s' has wrong position", m_pos.x, m_pos.y, m_world->name().c_str());
		return;
	}

	reader.read(m_heightMap, (MAP_SIZE + 1) * (MAP_SIZE + 1));
	reader.read(m_waterHeight, NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE);

	reader >> m_layerCount;
	m_layers = new Layer[m_layerCount];

	int textureId;

	for (int i = 0; i < m_layerCount; i++)
	{
		Layer& layer = m_layers[i];

		reader.read(layer.patchEnabled, NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE);

		reader >> textureId
			>> layer.lightMapOffset.x
			>> layer.lightMapOffset.y;

		layer.texture = TextureManager::getTerrainTexture(textureId);
	}

	reader >> m_lightMapSize.x
		>> m_lightMapSize.y;

	m_lightMapData = new u8vec4[m_lightMapSize.x * m_lightMapSize.y];
	reader.read(m_lightMapData, m_lightMapSize.x * m_lightMapSize.y);

	const ObjectType objTypes[] = {
		OT_OBJ,
		OT_SFX
	};

	int objCount, modelId;
	vec3 pos, rot, scale;
	Object* obj;

	const vec3 landObjOffset = ivec3(m_pos.x, 0, m_pos.y) * ShaderVars::MPU * MAP_SIZE;

	for (int i = 0; i < sizeof(objTypes) / sizeof(ObjectType); i++)
	{
		reader >> objCount;

		for (int j = 0; j < objCount; j++)
		{
			reader >> modelId
				>> pos
				>> rot
				>> scale;

			pos *= vec3(ShaderVars::MPU, 1, ShaderVars::MPU);
			pos += landObjOffset;

			obj = new Object(objTypes[i]);
			obj->setPos(pos);
			obj->setRot(rot);
			obj->setScale(scale);
			obj->setWorld(m_world);

			if (obj->setModelId(modelId) && insertObjLink(obj))
				addObjArray(obj);
			else
				delete obj;
		}

		m_objects[objTypes[i]].shrink_to_fit();
	}

	for (p.y = 0; p.y < NUM_PATCHES_PER_SIDE; p.y++)
		for (p.x = 0; p.x < NUM_PATCHES_PER_SIDE; p.x++)
			m_patches[p.y * NUM_PATCHES_PER_SIDE + p.x].init(m_heightMap, m_waterHeight[p.y * NUM_PATCHES_PER_SIDE + p.x], m_pos, p);

	calculateBounds();
	updateCull();

	if (gl::isContextActive())
		onContextRestored();
}

void Landscape::setVertices()
{
	const vec2 temp = vec2((float)(PATCH_SIZE - 1) / (float)(PATCH_SIZE)) / vec2(m_lightMapSize);
	int X, Y, i, j;

	m_waterVertexCount = 0;
	m_cloudVertexCount = 0;

	for (i = 0; i < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; i++)
	{
		const WaterHeight& w = m_waterHeight[i];
		if (w.type == WaterHeight::Water)
			m_waterVertexCount += 4;
		else if (w.type == WaterHeight::Cloud)
			m_cloudVertexCount += 4;
	}

	const int vertexCount = (PATCH_SIZE + 1) * (PATCH_SIZE + 1) * (NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE);
	const int dataSize = vertexCount * sizeof(TerrainVertex) + m_waterVertexCount * sizeof(WaterVertex) + m_cloudVertexCount * sizeof(CloudVertex);
	m_waterVertexOffset = vertexCount * sizeof(TerrainVertex);
	m_cloudVertexOffset = vertexCount * sizeof(TerrainVertex) + m_waterVertexCount * sizeof(WaterVertex);

	char* rawData = new char[dataSize];

	TerrainVertex* v = (TerrainVertex*)rawData;
	WaterVertex* wV = (WaterVertex*)(rawData + m_waterVertexOffset);
	CloudVertex* cV = (CloudVertex*)(rawData + m_cloudVertexOffset);

	const int MPU = ShaderVars::MPU;
	const int worldX = m_pos.x * MAP_SIZE;
	const int worldY = m_pos.y * MAP_SIZE;
	vec3 normal, v1, v2, v3, v4;

	for (Y = 0; Y < NUM_PATCHES_PER_SIDE; Y++)
	{
		for (X = 0; X < NUM_PATCHES_PER_SIDE; X++)
		{
			for (i = 0; i < PATCH_SIZE + 1; i++)
			{
				for (j = 0; j < PATCH_SIZE + 1; j++)
				{
					const float height = getHeightMap(((i + Y * PATCH_SIZE) * (MAP_SIZE + 1)) + (j + X * PATCH_SIZE));
					const vec3 tempPos = v->p = vec3((float)(X * PATCH_SIZE + j + worldX) * MPU, height, (float)(Y * PATCH_SIZE + i + worldY) * MPU);

					if ((j - 1 + X*PATCH_SIZE) > 0)
						v1 = vec3((float)(X*PATCH_SIZE + j - 1 + worldX) * MPU, getHeightMap(((i + Y*PATCH_SIZE)*(MAP_SIZE + 1)) + (j - 1 + X*PATCH_SIZE)), (float)(Y*PATCH_SIZE + i + worldY) * MPU);
					if ((i + 1 + Y*PATCH_SIZE) < MAP_SIZE + 1)
						v2 = vec3((float)(X*PATCH_SIZE + j + worldX) * MPU, getHeightMap(((i + 1 + Y*PATCH_SIZE)*(MAP_SIZE + 1)) + (j + X*PATCH_SIZE)), (float)(Y*PATCH_SIZE + i + 1 + worldY) * MPU);
					if ((j + 1 + X*PATCH_SIZE) < MAP_SIZE + 1)
						v3 = vec3((float)(X*PATCH_SIZE + j + 1 + worldX) * MPU, getHeightMap(((i + Y*PATCH_SIZE)*(MAP_SIZE + 1)) + (j + 1 + X*PATCH_SIZE)), (float)(Y*PATCH_SIZE + i + worldY) * MPU);
					if ((i - 1 + Y*PATCH_SIZE) > 0)
						v4 = vec3((float)(X*PATCH_SIZE + j + worldX) * MPU, getHeightMap(((i - 1 + Y*PATCH_SIZE)*(MAP_SIZE + 1)) + (j + X*PATCH_SIZE)), (float)(Y*PATCH_SIZE + i - 1 + worldY) * MPU);

					normal = vec3();
					if ((j - 1 + X*PATCH_SIZE) > 0 && (i + 1 + Y*PATCH_SIZE) < MAP_SIZE + 1)
						normal += cross(tempPos - v1, v1 - v2);
					if ((i + 1 + Y*PATCH_SIZE) < MAP_SIZE + 1 && (j + 1 + X*PATCH_SIZE) < MAP_SIZE + 1)
						normal += cross(tempPos - v2, v2 - v3);
					if ((j + 1 + X*PATCH_SIZE) < MAP_SIZE + 1 && (i - 1 + Y*PATCH_SIZE) > 0)
						normal += cross(tempPos - v3, v3 - v4);
					if ((i - 1 + Y*PATCH_SIZE) > 0 && (j - 1 + X*PATCH_SIZE) > 0)
						normal += cross(tempPos - v4, v4 - v1);
					v->n = normalize(normal);

					v->tu1 = ((float)j / (float)PATCH_SIZE) * 3.0f;
					v->tv1 = ((float)i / (float)PATCH_SIZE) * 3.0f;
					v->tu2 = (float)(X * PATCH_SIZE + j) * temp.x + (temp.x / 2.0f);
					v->tv2 = (float)(Y * PATCH_SIZE + i) * temp.y + (temp.y / 2.0f);

					v++;
				}
			}
		}
	}

	for (Y = 0; Y < NUM_PATCHES_PER_SIDE; Y++)
	{
		for (X = 0; X < NUM_PATCHES_PER_SIDE; X++)
		{
			const WaterHeight& w = m_waterHeight[Y * NUM_PATCHES_PER_SIDE + X];

			if (w.type != WaterHeight::None)
			{
				const vec2 minPatch = ((m_pos * MAP_SIZE) + (ivec2(X, Y) * PATCH_SIZE)) * ShaderVars::MPU;
				const vec2 maxPatch = minPatch + vec2(ivec2(PATCH_SIZE) * ShaderVars::MPU);

				if (w.type == WaterHeight::Water)
				{
					wV->p = vec3(minPatch.x, w.height, minPatch.y);
					wV->t = vec2(0, 0);
					wV->d = waterTextureColors[w.texture];
					wV++;

					wV->p = vec3(minPatch.x, w.height, maxPatch.y);
					wV->t = vec2(0, 3);
					wV->d = waterTextureColors[w.texture];
					wV++;

					wV->p = vec3(maxPatch.x, w.height, minPatch.y);
					wV->t = vec2(3, 0);
					wV->d = waterTextureColors[w.texture];
					wV++;

					wV->p = vec3(maxPatch.x, w.height, maxPatch.y);
					wV->t = vec2(3, 3);
					wV->d = waterTextureColors[w.texture];
					wV++;
				}
				else if (w.type == WaterHeight::Cloud)
				{
					cV->p = vec3(minPatch.x, w.height, minPatch.y);
					cV->t = vec2(0, 0);
					cV++;

					cV->p = vec3(minPatch.x, w.height, maxPatch.y);
					cV->t = vec2(0, 1);
					cV++;

					cV->p = vec3(maxPatch.x, w.height, minPatch.y);
					cV->t = vec2(1, 0);
					cV++;

					cV->p = vec3(maxPatch.x, w.height, maxPatch.y);
					cV->t = vec2(1, 1);
					cV++;
				}
			}
		}
	}

	m_VBO.create();
	m_VBO.bind();
	m_VBO.data(dataSize, rawData);
	delete[] rawData;

	m_VAO.create();
	m_VAO.bind();
	ShaderVars::terrainIBO.bind(m_VAO);
	m_VAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(TerrainVertex), 0);
	m_VAO.vertexAttribPointer<vec3>(VATTRIB_NORMAL, false, sizeof(TerrainVertex), sizeof(vec3));
	m_VAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(TerrainVertex), sizeof(vec3) * 2);
	m_VAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD1, false, sizeof(TerrainVertex), sizeof(vec3) * 2 + sizeof(vec2));

	if (m_waterVertexCount)
	{
		m_waterVAO.create();
		m_waterVAO.bind();
		ShaderVars::quadsIBO.bind(m_waterVAO);
		m_waterVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(WaterVertex), m_waterVertexOffset);
		m_waterVAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(WaterVertex), m_waterVertexOffset + sizeof(vec3));
		m_waterVAO.vertexAttribPointer<u8vec4>(VATTRIB_DIFFUSE, true, sizeof(WaterVertex), m_waterVertexOffset + sizeof(vec3) + sizeof(vec2));
	}

	if (m_cloudVertexCount)
	{
		m_cloudVAO.create();
		m_cloudVAO.bind();
		ShaderVars::quadsIBO.bind(m_cloudVAO);
		m_cloudVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(CloudVertex), m_cloudVertexOffset);
		m_cloudVAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(CloudVertex), m_cloudVertexOffset + sizeof(vec3));
	}
}

float Landscape::getHeightMap(uint offset) const
{
	const float height = m_heightMap[offset];

	if (height >= HGT_NOWALK)
	{
		if (height >= HGT_DIE)
			return m_heightMap[offset] - HGT_DIE;
		if (height >= HGT_NOMOVE)
			return m_heightMap[offset] - HGT_NOMOVE;
		if (height >= HGT_NOFLY)
			return m_heightMap[offset] - HGT_NOFLY;
		return m_heightMap[offset] - HGT_NOWALK;
	}

	return height;
}

void Landscape::updateCull()
{
	cull();

	if (m_visible)
	{
		for (int i = 0; i < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; i++)
			m_patches[i].cull();
	}
}

void Landscape::calculateBounds()
{
	float maxy = -9999999.0f;
	float miny = 9999999.0f;

	for (int i = 0; i < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; i++)
	{
		const Patch& patch = m_patches[i];

		if (patch.bounds[0].y < miny)
			miny = patch.bounds[0].y;
		if (patch.bounds[7].y > maxy)
			maxy = patch.bounds[7].y;
	}

	const ivec2 boundsMin = (m_pos * MAP_SIZE) * ShaderVars::MPU;
	const ivec2 boundsMax = boundsMin + (MAP_SIZE * ShaderVars::MPU);

	m_bounds[0] = vec3(boundsMin.x, miny, boundsMin.y); // xyz
	m_bounds[1] = vec3(boundsMax.x, miny, boundsMin.y); // Xyz
	m_bounds[2] = vec3(boundsMin.x, maxy, boundsMin.y); // xYz
	m_bounds[3] = vec3(boundsMax.x, maxy, boundsMin.y); // XYz
	m_bounds[4] = vec3(boundsMin.x, miny, boundsMax.y); // xyZ
	m_bounds[5] = vec3(boundsMax.x, miny, boundsMax.y); // XyZ
	m_bounds[6] = vec3(boundsMin.x, maxy, boundsMax.y); // xYZ
	m_bounds[7] = vec3(boundsMax.x, maxy, boundsMax.y); // XYZ
}

void Landscape::cull()
{
	static uint8_t outside[8];
	memset(outside, 0, sizeof(outside));

	int iPoint, iPlane;

	for (iPoint = 0; iPoint < 8; iPoint++)
	{
		for (iPlane = 0; iPlane < 6; iPlane++)
		{
			if (ShaderVars::frustum[iPlane].x * m_bounds[iPoint].x +
				ShaderVars::frustum[iPlane].y * m_bounds[iPoint].y +
				ShaderVars::frustum[iPlane].z * m_bounds[iPoint].z +
				ShaderVars::frustum[iPlane].w < 0)
			{
				outside[iPoint] |= (1 << iPlane);
			}
		}

		if (outside[iPoint] == 0)
		{
			m_visible = true;
			return;
		}
	}

	m_visible = (outside[0] & outside[1] & outside[2] & outside[3] & outside[4] & outside[5] & outside[6] & outside[7]) == 0;
}

float Landscape::getHeight_fast(float x, float z) const
{
	if (x < 0 || x > MAP_SIZE || z < 0 || z > MAP_SIZE)
		return 0.0f;
	else
		return getHeightMap(((int)x) + ((int)z) * (MAP_SIZE + 1));
}

const WaterHeight* Landscape::getWaterHeight(int x, int z) const
{
	return &m_waterHeight[x + z * NUM_PATCHES_PER_SIDE];
}

float Landscape::getHeight(float x, float z) const
{
	if (x < 0 || x > MAP_SIZE || z < 0 || z > MAP_SIZE)
		return 0.0f;

	const int px = (int)x;
	const int pz = (int)z;
	const float dx = x - px;
	const float dz = z - pz;

	const float y1 = getHeightMap(px + pz*(MAP_SIZE + 1));
	const float y2 = getHeightMap(px + 1 + pz*(MAP_SIZE + 1));
	const float y3 = getHeightMap(px + (pz + 1)*(MAP_SIZE + 1));
	const float y4 = getHeightMap(px + 1 + (pz + 1)*(MAP_SIZE + 1));

	return (y1*(1 - dx)*(1 - dz)) + (y2*dx*(1 - dz)) + (y3*(1 - dx)*dz) + (y4*dx*dz);
}

void Landscape::Patch::init(const float* landHeight, const WaterHeight& waterHeight, const ivec2& landPos, const ivec2& pos)
{
	indexOffset = (128 * 3) * (pos.y * NUM_PATCHES_PER_SIDE + pos.x) * sizeof(uint16_t);

	const float* heightMap = &landHeight[(pos.y * PATCH_SIZE) * (MAP_SIZE + 1) + (pos.x * PATCH_SIZE)];

	float maxy = -9999999.0f;
	float miny = 9999999.0f;
	int i, j;

	for (i = 0; i <= PATCH_SIZE; i++)
	{
		for (j = 0; j <= PATCH_SIZE; j++)
		{
			float y = heightMap[(i *(MAP_SIZE + 1)) + j];

			if (y >= HGT_NOWALK)
			{
				if (y >= HGT_DIE)
					y -= HGT_DIE;
				else if (y >= HGT_NOMOVE)
					y -= HGT_NOMOVE;
				else if (y >= HGT_NOFLY)
					y -= HGT_NOFLY;
				else
					y -= HGT_NOWALK;
			}

			if (y > maxy)
				maxy = y;
			if (y < miny)
				miny = y;
		}
	}

	if (waterHeight.type == WaterHeight::Cloud)
	{
		if (miny > waterHeight.height - 40.0f)
			miny = waterHeight.height - 40.0f;
		if (maxy < waterHeight.height)
			maxy = waterHeight.height;
	}
	else if (waterHeight.type == WaterHeight::Water)
	{
		if (miny > waterHeight.height)
			miny = waterHeight.height;
		if (maxy < waterHeight.height)
			maxy = waterHeight.height;
	}

	const ivec2 boundsMin = ((landPos * MAP_SIZE) + (pos * PATCH_SIZE)) * ShaderVars::MPU;
	const ivec2 boundsMax = boundsMin + (PATCH_SIZE * ShaderVars::MPU);

	bounds[0] = vec3(boundsMin.x, miny, boundsMin.y); // xyz
	bounds[1] = vec3(boundsMax.x, miny, boundsMin.y); // Xyz
	bounds[2] = vec3(boundsMin.x, maxy, boundsMin.y); // xYz
	bounds[3] = vec3(boundsMax.x, maxy, boundsMin.y); // XYz
	bounds[4] = vec3(boundsMin.x, miny, boundsMax.y); // xyZ
	bounds[5] = vec3(boundsMax.x, miny, boundsMax.y); // XyZ
	bounds[6] = vec3(boundsMin.x, maxy, boundsMax.y); // xYZ
	bounds[7] = vec3(boundsMax.x, maxy, boundsMax.y); // XYZ

	visible = false;
}

void Landscape::Patch::cull()
{
	static uint8_t outside[8];
	memset(outside, 0, sizeof(outside));

	int iPoint, iPlane;

	for (iPoint = 0; iPoint < 8; iPoint++)
	{
		for (iPlane = 0; iPlane < 6; iPlane++)
		{
			if (ShaderVars::frustum[iPlane].x * bounds[iPoint].x +
				ShaderVars::frustum[iPlane].y * bounds[iPoint].y +
				ShaderVars::frustum[iPlane].z * bounds[iPoint].z +
				ShaderVars::frustum[iPlane].w < 0)
			{
				outside[iPoint] |= (1 << iPlane);
			}
		}

		if (outside[iPoint] == 0)
		{
			visible = true;
			return;
		}
	}

	visible = (outside[0] & outside[1] & outside[2] & outside[3] & outside[4] & outside[5] & outside[6] & outside[7]) == 0;
}