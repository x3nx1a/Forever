#include "StdAfx.hpp"
#include "Object.hpp"
#include "ModelManager.hpp"
#include "GeometryUtils.hpp"
#include "World.hpp"
#include "Mesh.hpp"
#include "SfxModel.hpp"
#include "Config.hpp"

namespace
{
	const float minDistant[4] = { 100, 70, 40, 128 };
	const float maxDistant[4] = { 400, 200, 70, 400 };
	const float factorDistant[4] = { maxDistant[0] - minDistant[0], maxDistant[1] - minDistant[1], maxDistant[2] - minDistant[2], maxDistant[3] - minDistant[3] };

	const float objectQuality[3] = { 15, 30, 50 };
}

bool Object::sortFarToNear(const Object* obj1, const Object* obj2)
{
	return obj1->m_distToCamera > obj2->m_distToCamera;
}

Object::Object(ObjectType type)
	: m_type(type),
	m_world(nullptr),
	m_model(nullptr),
	m_updateMatrix(true),
	m_visible(false),
	m_distToCamera(99999.9f),
	m_scale(1),
	m_objFlags(0)
{
}

Object::~Object()
{
}

void Object::render()
{
	if (m_model->modelType() == MODELTYPE_SFX)
		((SfxModel*)m_model.get())->render(m_pos + vec3(0.0f, 0.2f, 0.0f), m_rot, m_scale);
	else
	{
		int lod = 0;

		if (!m_world->inDoor())
			lod = glm::min(2, (int)(m_distToCamera / objectQuality[Config::objectQuality]));

		((Mesh*)m_model.get())->render(m_TM, lod);
	}
}

bool Object::setModelId(int modelId)
{
	m_model = ModelManager::createModel(m_type, modelId);
	if (!m_model)
		return false;

	return true;
}

void Object::setPos(const vec3& p)
{
	if (m_pos != p)
	{
		m_pos = p;
		m_updateMatrix = true;
	}
}

void Object::setRot(const vec3& r)
{
	if (m_rot != r)
	{
		m_rot = r;
		m_updateMatrix = true;
	}
}

void Object::setScale(const vec3& s)
{
	if (m_scale != s)
	{
		m_scale = s;
		m_updateMatrix = true;
	}
}

void Object::setWorld(World* world)
{
	m_world = world;
}

void Object::update(int frameCount)
{
	if (!m_model->loaded())
		return;

	if (m_model->modelType() == MODELTYPE_SFX)
		((SfxModel*)m_model.get())->update(frameCount);
	else
		((Mesh*)m_model.get())->update(m_pos, frameCount);
}

void Object::setFlag(uint32_t flag, bool val)
{
	if (val)
		m_objFlags |= flag;
	else
		m_objFlags &= ~flag;
}

void Object::cull()
{
	if (!m_model->loaded())
		return;

	if (m_updateMatrix)
		updateMatrix();

	m_distToCamera = length(ShaderVars::cameraPos - m_pos);

	const uint8_t distant = m_model->prop()->distant;
	if (m_distToCamera > (minDistant[distant] + Config::fieldViewFactor * factorDistant[distant]))
	{
		m_visible = false;
		return;
	}

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

void Object::updateMatrix()
{
	const mat4 matScale = scale(mat4(), m_scale);
	const mat4 matRot = yawPitchRoll(radians(-m_rot.y), radians(-m_rot.x), radians(m_rot.z));
	const mat4 matPos = translate(mat4(), m_pos);
	m_TM = matPos * matScale * matRot;

	vec3 bbMin, bbMax;
	m_model->bounds(bbMin, bbMax);

	m_bounds[0].x = bbMin.x;
	m_bounds[0].y = bbMax.y;
	m_bounds[0].z = bbMin.z;
	m_bounds[1].x = bbMax.x;
	m_bounds[1].y = bbMax.y;
	m_bounds[1].z = bbMin.z;
	m_bounds[2].x = bbMax.x;
	m_bounds[2].y = bbMax.y;
	m_bounds[2].z = bbMax.z;
	m_bounds[3].x = bbMin.x;
	m_bounds[3].y = bbMax.y;
	m_bounds[3].z = bbMax.z;
	m_bounds[4].x = bbMin.x;
	m_bounds[4].y = bbMin.y;
	m_bounds[4].z = bbMin.z;
	m_bounds[5].x = bbMax.x;
	m_bounds[5].y = bbMin.y;
	m_bounds[5].z = bbMin.z;
	m_bounds[6].x = bbMax.x;
	m_bounds[6].y = bbMin.y;
	m_bounds[6].z = bbMax.z;
	m_bounds[7].x = bbMin.x;
	m_bounds[7].y = bbMin.y;
	m_bounds[7].z = bbMax.z;

	for (int i = 0; i < 8; i++)
		m_bounds[i] = transformCoord(m_bounds[i], m_TM);

	m_updateMatrix = false;
}