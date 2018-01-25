#pragma once

#include "Object.hpp"

class Sfx : public Object
{
public:
	explicit Sfx();

public:
	virtual void update(int frameCount);

private:
	int m_maxPlay;
	int m_playCount;

public:
	static Sfx* create(World* world, int sfxId, const vec3& pos);
};