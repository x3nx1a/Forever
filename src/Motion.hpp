#pragma once

#include "BinaryReader.hpp"

struct MotionAttribute
{
	enum Type
	{
		Hit = 1 << 0,
		Sound = 1 << 1,
		Quake = 1 << 2
	};

	uint32_t type;
	int soundId;
};

struct Bone
{
	int parentId;
	mat4 TM;
	mat4 inverseTM;
};

struct TMAnimation
{
	quat rot;
	vec3 pos;
};

struct BoneFrame
{
	TMAnimation* frames;
	mat4 TM;
};

class Skeleton;

class Motion
{
public:
	explicit Motion(const char* name);
	~Motion();

	void load(BinaryReader& reader, uint8_t ver);

	const char* name() const {
		return m_name;
	}
	int frameCount() const {
		return m_frameCount;
	}

private:
	char m_name[32];
	int m_frameCount;
	int m_boneCount;
	BoneFrame* m_frames;
	MotionAttribute* m_attributes;
	TMAnimation* m_anis;

	friend class Skeleton;

private:
	Motion(const Motion&) = delete;
	Motion& operator=(const Motion&) = delete;
};

class Skeleton
{
public:
	explicit Skeleton();
	~Skeleton();

	void load(BinaryReader& reader, uint8_t ver);

	mat4* createBones() const;
	void resetBones(mat4* bones) const;

	void animate(Motion* motion, mat4* bones, float currentFrame, int nextFrame);
	bool sendVS() const {
		return m_sendVS;
	}

	void sendSkinBones(const mat4* bones, const int* boneList, int boneCount) const;

private:
	int m_boneCount;
	Bone* m_bones;
	bool m_sendVS;
	int m_skinBoneCount;

private:
	Skeleton(const Skeleton&) = delete;
	Skeleton& operator=(const Skeleton&) = delete;
};