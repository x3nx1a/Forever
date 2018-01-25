#pragma once

#include "Model.hpp"
#include "ModelFile.hpp"

class SfxPart;
class SfxPartParticle;

struct SfxParticle
{
	vec3 pos;
	vec3 speed;
	vec3 scale;
	vec3 rotation;
	vec3 scaleSpeed;
	int frame;
	bool scal;

	typedef vector<SfxParticle> Array; // TODO : tests vector vs list
};

class SfxModel : public Model
{
public:
	SfxModel(const ModelProp* prop);
	virtual ~SfxModel();

	void load(const string& filename);
	void render(const vec3& pos, const vec3& angle, const vec3& scale) const;
	void update(int frameCount);

protected:
	virtual bool checkLoaded();

private:
	void renderParticles(const SfxParticle::Array& particles, const SfxPartParticle* part, const vec3& pos, const vec3& angle, const vec3& scale) const;

private:
	ModelFilePtr m_file;
	int m_partCount;
	SfxPart** m_parts;
	SfxParticle::Array** m_particles;
};