#include "StdAfx.hpp"
#include "World.hpp"
#include "Skybox.hpp"
#include "TextureManager.hpp"

World::World(const string& name)
	: Resource("world/" + name + "/properties.bin"),
	m_MPU(0),
	m_updateView(true),
	m_lands(nullptr),
	m_visibilityLand(0),
	m_farPlane(0.0f),
	m_fogStart(70.0f),
	m_fogEnd(400.0f),
	m_waterFrame(0.0f),
	m_cullLandCount(0),
	m_skybox(nullptr),
	m_inDoor(false),
	m_name(name),
	m_cullObjCount(0),
	m_cullSfxCount(0),
	m_weather(WEATHER_NONE)
{
	startLoad();
}

World::~World()
{
	if (m_lands)
		delete[] m_lands;
	if (m_skybox)
		delete m_skybox;
}

void World::update(int frameCount)
{
	if (!loaded())
		return;

	if (m_updateView)
	{
		updateView();
		m_updateView = false;
	}

	m_waterFrame = mod(m_waterFrame + 0.15f * frameCount, 16.0f);
	m_cloudsPos[0] = mod(m_cloudsPos[0] + 0.001f * frameCount, 1.0f);
	m_cloudsPos[1] = mod(m_cloudsPos[1] + 0.0015f * frameCount, 1.0f);

	if (m_skybox)
		m_skybox->update(frameCount);

	const float maxDistToCamera = glm::max(150.0f, m_farPlane / 2.0f);

	int i, type;
	std::size_t j;
	for (i = 0; i < m_cullLandCount; i++)
	{
		for (type = OT_ANI; type < MAX_OBJTYPE; type++)
		{
			const vector<Object*>& objects = m_cullLands[i]->objects((ObjectType)type);
			for (j = 0; j < objects.size(); j++)
			{
				Object* const obj = objects[j];

				if (!obj->isDelete() && obj->distToCamera() < maxDistToCamera)
					obj->update(frameCount);
			}
		}
	}

	for (std::size_t i = 0; i < m_deleteObjs.size(); i++)
	{
		Object* const obj = m_deleteObjs[i];
		if (obj)
		{
			removeObjLink(obj);
			removeObjArray(obj);
			delete obj;
		}
	}
	m_deleteObjs.clear();

	cullObjects();
}

void World::onLoad(BinaryReader reader)
{
	const uint8_t ver = reader.read<uint8_t>();

	reader >> m_size.x
		>> m_size.y
		>> m_MPU
		>> m_fogStart
		>> m_fogEnd
		>> m_inDoor
		>> m_ambient
		>> m_diffuse
		>> m_lightDir;

	m_lands = new LandscapePtr[m_size.x * m_size.y];

	m_cloudTexture = TextureManager::getTerrainTexture(10);

	if (!m_inDoor)
		m_skybox = new Skybox(this);
}

bool World::addObject(Object* obj)
{
	if (obj->world() || !vecInWorld(obj->pos()))
		return false;

	obj->setWorld(this);

	if (!insertObjLink(obj))
		return false;

	addObjArray(obj);
	return true;
}

void World::deleteObject(Object* obj)
{
	if (obj->isDelete())
		return;

	obj->setDelete(true);
	m_deleteObjs.push_back(obj);
}

bool World::insertObjLink(Object* obj)
{
	return true;
}

bool World::removeObjLink(Object* obj)
{
	return true;
}

void World::addObjArray(Object* obj)
{
	Landscape* const land = getLandscape(obj->pos());
	if (land)
		land->addObjArray(obj);
}

void World::removeObjArray(Object* obj)
{
	Landscape* const land = getLandscape(obj->pos());
	if (land)
		land->removeObjArray(obj);
}

float World::getLandHeight_fast(float x, float z) const
{
	if (!vecInWorld(x, z))
		return 0.0f;

	x /= m_MPU;
	z /= m_MPU;
	const int mX = (int)(x / MAP_SIZE);
	const int mZ = (int)(z / MAP_SIZE);

	const Landscape* const land = m_lands[mX + mZ * m_size.x].get();
	if (!land)
		return 0.0f;

	return land->getHeight_fast(x - (mX * MAP_SIZE), z - (mZ * MAP_SIZE));
}

float World::getLandHeight(float x, float z) const
{
	if (!vecInWorld(x, z))
		return 0.0f;

	x /= m_MPU;
	z /= m_MPU;
	const int mX = (int)(x / MAP_SIZE);
	const int mZ = (int)(z / MAP_SIZE);

	const Landscape* const land = m_lands[mX + mZ * m_size.x].get();
	if (!land)
		return 0.0f;

	return land->getHeight(x - (mX * MAP_SIZE), z - (mZ * MAP_SIZE));
}

const WaterHeight* World::getWaterHeight(int x, int z) const
{
	if (!vecInWorld((float)x, (float)z))
		return nullptr;

	x /= m_MPU;
	z /= m_MPU;
	const int mX = (int)(x / MAP_SIZE);
	const int mZ = (int)(z / MAP_SIZE);

	const Landscape* const land = m_lands[mX + mZ * m_size.x].get();
	if (!land)
		return nullptr;

	return land->getWaterHeight((x % MAP_SIZE) / PATCH_SIZE, (z % MAP_SIZE) / PATCH_SIZE);
}

Landscape* World::getLandscape(const vec3& p) const
{
	if (!vecInWorld(p.x, p.z))
		return nullptr;

	const float x = p.x / m_MPU;
	const float z = p.z / m_MPU;
	const int mX = (int)(x / MAP_SIZE);
	const int mZ = (int)(z / MAP_SIZE);

	return m_lands[mX + mZ * m_size.x].get();
}

void World::getLandTri(float x, float z, vec3* out) const
{
	if (!vecInWorld(x, z))
		return;

	x /= m_MPU;
	z /= m_MPU;

	const int lx = (int)(x / MAP_SIZE);
	const int lz = (int)(z / MAP_SIZE);
	const int gx = (int)x - lx*MAP_SIZE;
	const int gz = (int)z - lz*MAP_SIZE;
	const float glx = x - (int)x;
	const float glz = z - (int)z;

	Landscape* const land = m_lands[lx + lz * m_size.x].get();
	if (!land)
		return;

	if ((gx + gz) % 2 == 0)
	{
		if (glx > glz)
		{
			//0,3,1
			out[0] = vec3((float)((gx + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx, gz)), (float)((gz + lz*MAP_SIZE)*m_MPU));
			out[1] = vec3((float)((gx + 1 + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx + 1, gz + 1)), (float)((gz + 1 + lz*MAP_SIZE)*m_MPU));
			out[2] = vec3((float)((gx + 1 + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx + 1, gz)), (float)((gz + lz*MAP_SIZE)*m_MPU));
		}
		else
		{
			//0,2,3
			out[0] = vec3((float)((gx + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx, gz)), (float)((gz + lz*MAP_SIZE)*m_MPU));
			out[1] = vec3((float)((gx + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx, gz + 1)), (float)((gz + 1 + lz*MAP_SIZE)*m_MPU));
			out[2] = vec3((float)((gx + 1 + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx + 1, gz + 1)), (float)((gz + 1 + lz*MAP_SIZE)*m_MPU));
		}
	}
	else
	{
		if (glx + glz < 1.0f)
		{
			//0,2,1
			out[0] = vec3((float)((gx + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx, gz)), (float)((gz + lz*MAP_SIZE)*m_MPU));
			out[1] = vec3((float)((gx + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx, gz + 1)), (float)((gz + 1 + lz*MAP_SIZE)*m_MPU));
			out[2] = vec3((float)((gx + 1 + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx + 1, gz)), (float)((gz + lz*MAP_SIZE)*m_MPU));
		}
		else
		{
			//1,2,3
			out[0] = vec3((float)((gx + 1 + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx + 1, gz)), (float)((gz + lz*MAP_SIZE)*m_MPU));
			out[1] = vec3((float)((gx + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx, gz + 1)), (float)((gz + 1 + lz*MAP_SIZE)*m_MPU));
			out[2] = vec3((float)((gx + 1 + lx*MAP_SIZE)*m_MPU), (float)(land->getHeight(gx + 1, gz + 1)), (float)((gz + 1 + lz*MAP_SIZE)*m_MPU));
		}
	}
}

vec3 World::getLandNormal(float x, float z) const
{
	vec3 tri[3];
	getLandTri(x, z, tri);
	return normalize(cross(tri[2] - tri[0], tri[1] - tri[0]));
}

vec3 World::getLandRot(float x, float z) const
{
	const vec3 normal = getLandNormal(x, z);
	const vec3 up(0, -1, 0);

	return eulerAngles(angleAxis(acos(dot(up, normal)), cross(up, normal)));
}