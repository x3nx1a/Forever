#pragma once

#include "Model.hpp"

class World;

class Object
{
public:
	enum ObjFlags
	{
		Delete = 1 << 0
	};

public:
	explicit Object(ObjectType type);
	virtual ~Object();

	bool setModelId(int modelId);
	void setPos(const vec3& p);
	void setRot(const vec3& r);
	void setScale(const vec3& s);
	void setWorld(World* world);

	void cull();
	void updateMatrix();

	World* world() const {
		return m_world;
	}
	Model* model() {
		return m_model.get();
	}
	ObjectType type() const {
		return m_type;
	}
	bool visible() const {
		return m_visible;
	}
	const vec3& pos() const {
		return m_pos;
	}
	const float distToCamera() const {
		return m_distToCamera;
	}

	bool hasObjFlag(uint32_t flag) const {
		return (m_objFlags & flag) != 0;
	}
	void setFlag(uint32_t flag, bool val);

	bool isDelete() const {
		return hasObjFlag(Delete);
	}
	void setDelete(bool val) {
		setFlag(Delete, val);
	}

public:
	virtual void render();
	virtual void update(int frameCount);

protected:
	ObjectType m_type;
	World* m_world;
	vec3 m_pos;
	vec3 m_scale;
	vec3 m_rot;
	ModelPtr m_model;
	bool m_updateMatrix;
	mat4 m_TM;
	vec3 m_bounds[8];
	bool m_visible;
	float m_distToCamera;
	uint32_t m_objFlags;

public:
	static bool sortFarToNear(const Object*, const Object*);

private:
	Object(const Object&) = delete;
	Object& operator=(const Object&) = delete;
};

inline bool isValidObject(Object* obj)
{
	return obj && obj->world() != nullptr && !obj->isDelete();
}