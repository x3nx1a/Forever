#include "StdAfx.hpp"
#include "Sfx.hpp"
#include "World.hpp"

Sfx* Sfx::create(World* world, int sfxId, const vec3& pos)
{
	Sfx* sfx = nullptr;

	switch (sfxId)
	{
	default:
		sfx = new Sfx();
		break;
	}

	sfx->setPos(pos);

	if (!sfx->setModelId(sfxId) || !world->addObject(sfx))
	{
		delete sfx;
		sfx = nullptr;
	}

	return sfx;
}

Sfx::Sfx()
	: Object(OT_SFX),
	m_playCount(0),
	m_maxPlay(1)
{
}

void Sfx::update(int frameCount)
{
	Object::update(frameCount);

	if (m_model->isEndFrame())
	{
		m_playCount++;

		if (m_maxPlay > 0 && m_playCount >= m_maxPlay)
			m_world->deleteObject(this);
	}
}