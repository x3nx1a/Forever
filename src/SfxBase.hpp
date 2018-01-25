#pragma once

#include "BinaryReader.hpp"
#include "ModelFile.hpp"
#include "Texture.hpp"

enum class SfxPartType : uint8_t
{
	Bill = 1,
	Particle = 2,
	Mesh = 3,
	CustomMesh = 4,
};

enum class SfxPartBillType : uint8_t
{
	Bill = 1,
	Bottom = 2,
	Pole = 3,
	Normal = 4,
};

enum class SfxPartAlphaType : uint8_t
{
	Blend = 1,
	Glow = 2,
};

struct SfxKeyFrame
{
	vec3 pos;
	vec3 posRotate;
	vec3 scale;
	vec3 rotate;
	float alpha;
	int frame;
};

class SfxModel;

class SfxPart
{
public:
	SfxPart();
	virtual ~SfxPart();

	virtual void load(BinaryReader& reader, uint8_t ver);
	virtual void render(int frame, const vec3& pos, const vec3& angle, const vec3& scale) = 0;

	virtual SfxPartType type() const = 0;

	const SfxKeyFrame* getKey(int frame) const;
	const bool getKey(int frame, SfxKeyFrame& key) const;
	const SfxKeyFrame* getPrevKey(int frame) const;
	const SfxKeyFrame* getNextKey(int frame, bool skip = true) const;
	const SfxKeyFrame* getFirstKey() const;
	const SfxKeyFrame* getLastKey() const;

protected:
	virtual void setTexture();

protected:
	SfxPartBillType m_billType;
	SfxPartAlphaType m_alphaType;
	int m_texFrame;
	int m_texLoop;
	char m_textureName[128];
	int m_keyCount;
	SfxKeyFrame* m_keys;
	TexturePtr m_texture;
	TexturePtr* m_textures;
	bool m_hasTexture;
};

class SfxPartBill : public SfxPart
{
public:
	SfxPartBill();

	virtual SfxPartType type() const {
		return SfxPartType::Bill;
	}

	virtual void render(int frame, const vec3& pos, const vec3& angle, const vec3& scale);
};

class SfxPartMesh : public SfxPart
{
public:
	SfxPartMesh();
	virtual ~SfxPartMesh();

	virtual SfxPartType type() const {
		return SfxPartType::Mesh;
	}

	virtual void render(int frame, const vec3& pos, const vec3& angle, const vec3& scale);

protected:
	virtual void setTexture();

private:
	Object3D* m_obj3d;
	mat4* m_bones;
	ModelFilePtr m_modelFile;
};

class SfxPartCustomMesh : public SfxPart
{
public:
	SfxPartCustomMesh();

	virtual void load(BinaryReader& reader, uint8_t ver);

	virtual SfxPartType type() const {
		return SfxPartType::CustomMesh;
	}

	virtual void render(int frame, const vec3& pos, const vec3& angle, const vec3& scale);

private:
	int m_pointCount;
};

class SfxPartParticle : public SfxPart
{
public:
	SfxPartParticle();

	virtual void load(BinaryReader& reader, uint8_t ver);

	virtual SfxPartType type() const {
		return SfxPartType::Particle;
	}

	virtual void render(int frame, const vec3& pos, const vec3& angle, const vec3& scale);

private:
	int m_particleCreate;
	int m_particleCreateNum;
	int m_particleFrameAppear;
	int m_particleFrameKeep;
	int m_particleFrameDisappear;
	float m_particleStartPosVar;
	float m_particleStartPosVarY;
	float m_particleYLow;
	float m_particleYHigh;
	float m_particleXZLow;
	float m_particleXZHigh;
	vec3 m_particleAccel;
	vec3 m_scale;
	vec3 m_scaleSpeed;
	vec3 m_rotation;
	vec3 m_rotationLow;
	vec3 m_rotationHigh;
	bool m_repeatScal;
	float m_scalSpeedXLow;
	float m_scalSpeedXHigh;
	float m_scalSpeedYLow;
	float m_scalSpeedYHigh;
	float m_scalSpeedZLow;
	float m_scalSpeedZHigh;
	vec3 m_scaleSpeed2;
	vec3 m_scaleEnd;
	bool m_repeat;

	friend class SfxModel;
};

class SfxBase
{
public:
	explicit SfxBase();
	~SfxBase();

	void load(BinaryReader& reader, uint8_t ver);

	int partCount() const {
		return m_partCount;
	}
	SfxPart** const parts() const {
		return m_parts;
	}
	void bounds(vec3& bbMin, vec3& bbMax) const {
		bbMin = m_bbMin;
		bbMax = m_bbMax;
	}
	int frameCount() const {
		return m_frameCount;
	}

private:
	int m_partCount;
	SfxPart** m_parts;
	vec3 m_bbMin, m_bbMax;
	int m_frameCount;
};