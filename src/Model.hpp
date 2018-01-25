#pragma once

#include "RefCounted.hpp"

enum ObjectType
{
	OT_OBJ,
	OT_ANI,
	OT_CTRL,
	OT_SFX,
	MAX_OBJTYPE
};

enum ModelType
{
	MODELTYPE_NONE = 0,
	MODELTYPE_MESH = 1,
	MODELTYPE_ANIMATED_MESH = 2,
	MODELTYPE_SFX = 4
};

struct ModelProp
{
	uint8_t modelType;
	int id;
	char filename[64];
	uint8_t distant;
};

class Model : public RefCounted
{
public:
	explicit Model(const ModelProp* prop);

	const ModelProp* prop() const {
		return m_prop;
	}

	bool loaded() {
		if (m_loaded)
			return true;
		else if (checkLoaded())
		{
			m_loaded = true;
			return true;
		}
		else
			return false;
	}
	void bounds(vec3& bbMin, vec3& bbMax) const {
		bbMin = m_bbMin;
		bbMax = m_bbMax;
	}
	bool isAnimated() const {
		return m_prop->modelType != MODELTYPE_MESH;
	}
	ModelType modelType() const {
		return (ModelType)m_prop->modelType;
	}
	float currentFrame() const {
		return m_currentFrame;
	}
	bool isEndFrame() const {
		return m_endFrame;
	}

	int nextFrame() const
	{
		int next = (int)m_currentFrame;
		if (next >= m_frameCount)
			next = 0;
		return next;
	}

protected:
	virtual bool checkLoaded() = 0;

protected:
	const ModelProp* m_prop;
	bool m_loaded;
	vec3 m_bbMin, m_bbMax;
	int m_frameCount;
	float m_currentFrame;
	bool m_endFrame;
};

typedef RefCountedPtr<Model> ModelPtr;