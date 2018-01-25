#pragma once

#include "Resource.hpp"
#include "Motion.hpp"

class Object3D;
class SfxBase;

class ModelFile : public Resource
{
public:
	explicit ModelFile(const string& filename);
	virtual ~ModelFile();

	Object3D* object() {
		return m_obj;
	}
	Skeleton* skeleton() {
		return m_skel;
	}
	Motion* motion(const char* name = "") {
		for (std::size_t i = 0; i < m_motions.size(); i++)
			if (strcmp(m_motions[i]->name(), name) == 0)
				return m_motions[i];
		return nullptr;
	}
	SfxBase* sfx() {
		return m_sfx;
	}

protected:
	virtual void onLoad(BinaryReader reader);

private:
	Object3D* m_obj;
	Skeleton* m_skel;
	vector<Motion*> m_motions;
	SfxBase* m_sfx;
};

typedef RefCountedPtr<ModelFile> ModelFilePtr;