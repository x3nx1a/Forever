#pragma once

#include "Model.hpp"
#include "ModelFile.hpp"

#define MAX_MESH_ELEMENTS 21

class Mesh : public Model
{
public:
	Mesh(const ModelProp* prop);
	virtual ~Mesh();

	void loadPart(const string& filename, int part = 0);
	void render(const mat4& world, int lod) const;
	void update(const vec3& pos, int frameCount);

protected:
	virtual bool checkLoaded();

private:
	struct Part
	{
		ModelFilePtr file;
		Object3D* obj;
	};

	Part m_parts[MAX_MESH_ELEMENTS];
	mat4* m_bones;
	Skeleton* m_skeleton;
	int m_maxPart;
	Motion* m_motion;
};