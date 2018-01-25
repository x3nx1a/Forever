#include "StdAfx.hpp"
#include "Mesh.hpp"
#include "ModelManager.hpp"
#include "Shaders.hpp"
#include "Object3D.hpp"

Mesh::Mesh(const ModelProp* prop)
	: Model(prop),
	m_maxPart(0),
	m_bones(nullptr),
	m_skeleton(nullptr),
	m_motion(nullptr)
{
	for (int p = 0; p < MAX_MESH_ELEMENTS; p++)
		m_parts[p].obj = nullptr;
}

Mesh::~Mesh()
{
	if (m_bones)
		delete[] m_bones;
}

void Mesh::loadPart(const string& filename, int part)
{
	if (part < 0 || part >= MAX_MESH_ELEMENTS)
		return;

	m_parts[part].obj = nullptr;
	m_parts[part].file = ModelManager::getModelFile(filename);

	if (part + 1 > m_maxPart)
		m_maxPart = part + 1;
}

void Mesh::update(const vec3& pos, int frameCount)
{
	if (!m_loaded)
		return;

	if (m_motion)
	{
		m_currentFrame = mod(m_currentFrame + 0.5f * frameCount, (float)m_frameCount);

		m_skeleton->animate(m_motion, m_bones, m_currentFrame, nextFrame());
	}
}

void Mesh::render(const mat4& world, int lod) const
{
	if (m_bones && m_skeleton->sendVS())
	{
		Shaders::skin.use();
		m_skeleton->sendSkinBones(m_bones, nullptr, 0);
	}

	for (int p = 0; p < m_maxPart; p++)
	{
		const Part& part = m_parts[p];

		if (part.obj)
			part.obj->render(m_bones, world, lod, 0, 0, 1.0f);
	}
}

bool Mesh::checkLoaded()
{
	for (int p = 0; p < m_maxPart; p++)
		if (m_parts[p].file && !m_parts[p].file->loaded())
			return false;

	vec3 bbMin(99999.9f), bbMax(-99999.9f), bbMin2, bbMax2;

	for (int p = 0; p < m_maxPart; p++)
	{
		Part& part = m_parts[p];
		if (!part.file)
			continue;

		if (!part.obj)
		{
			part.obj = part.file->object();
			part.obj->loadTextureEx(0);
		}

		part.obj->bounds(bbMin2, bbMax2);

		if (bbMin2.x < bbMin.x) bbMin.x = bbMin2.x;
		if (bbMin2.y < bbMin.y) bbMin.y = bbMin2.y;
		if (bbMin2.z < bbMin.z) bbMin.z = bbMin2.z;
		if (bbMax2.x > bbMax.x) bbMax.x = bbMax2.x;
		if (bbMax2.y > bbMax.y) bbMax.y = bbMax2.y;
		if (bbMax2.z > bbMax.z) bbMax.z = bbMax2.z;

		if (!m_skeleton)
		{
			m_skeleton = part.file->skeleton();

			if (m_skeleton)
				m_bones = m_skeleton->createBones();

			m_motion = part.file->motion();
			if (m_motion)
				m_frameCount = m_motion->frameCount();
		}
	}

	m_bbMin = bbMin;
	m_bbMax = bbMax;

	m_loaded = true;
	return true;
}