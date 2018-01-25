#include "StdAfx.hpp"
#include "Motion.hpp"
#include "Shaders.hpp"

Motion::Motion(const char* name)
	: m_frameCount(0),
	m_boneCount(0),
	m_attributes(nullptr),
	m_frames(nullptr),
	m_anis(nullptr)
{
	strcpy(m_name, name);
}

Motion::~Motion()
{
	if (m_anis)
		delete[] m_anis;
	if (m_frames)
		delete[] m_frames;
	if (m_attributes)
		delete[] m_attributes;
}

void Motion::load(BinaryReader& reader, uint8_t ver)
{
	reader >> m_frameCount
		>> m_boneCount;

	const int aniCount = reader.read<int>();

	m_frames = new BoneFrame[m_boneCount];
	m_attributes = new MotionAttribute[m_frameCount];
	m_anis = new TMAnimation[aniCount];

	TMAnimation* ani = m_anis;
	int debug = 0;

	for (int i = 0; i < m_boneCount; i++)
	{
		BoneFrame& frame = m_frames[i];

		const bool hasFrames = reader.read<bool>();

		if (hasFrames)
		{
			reader.read(ani, m_frameCount);
			frame.frames = ani;

			ani += m_frameCount;
			debug += m_frameCount;
		}
		else
		{
			reader >> frame.TM;
			frame.frames = nullptr;
		}
	}

	for (int i = 0; i < m_frameCount; i++)
	{
		MotionAttribute& attrib = m_attributes[i];

		reader >> attrib.type
			>> attrib.soundId;
	}
}

Skeleton::Skeleton()
	: m_bones(nullptr),
	m_boneCount(0),
	m_sendVS(false),
	m_skinBoneCount(0)
{
}

Skeleton::~Skeleton()
{
	if (m_bones)
		delete[] m_bones;
}

void Skeleton::load(BinaryReader& reader, uint8_t ver)
{
	reader >> m_boneCount;

	m_bones = new Bone[m_boneCount];

	for (int i = 0; i < m_boneCount; i++)
	{
		Bone& bone = m_bones[i];

		reader >> bone.parentId
			>> bone.TM
			>> bone.inverseTM;
	}

	reader >> m_sendVS
		>> m_skinBoneCount;
}

mat4* Skeleton::createBones() const
{
	mat4* bones = new mat4[m_boneCount];
	resetBones(bones);
	return bones;
}

void Skeleton::resetBones(mat4* bones) const
{
	for (int i = 0; i < m_boneCount; i++)
		bones[i] = m_bones[i].TM;
}

void Skeleton::animate(Motion* motion, mat4* bones, float currentFrame_, int nextFrame)
{
	const BoneFrame* const frames = motion->m_frames;
	const int currentFrame = (int)currentFrame_;
	const float slp = currentFrame_ - (float)currentFrame;

	mat4 m;

	for (int i = 0; i < m_boneCount; i++)
	{
		if (frames[i].frames)
		{
			const TMAnimation& frame = frames[i].frames[currentFrame];
			const TMAnimation& next = frames[i].frames[nextFrame];

			m = translate(mat4(), lerp(frame.pos, next.pos, slp)) * mat4_cast(lerp(frame.rot, next.rot, slp));
		}
		else
			m = frames[i].TM;

		if (m_bones[i].parentId != -1)
			m = bones[m_bones[i].parentId] * m;

		bones[i] = m;
	}
}

void Skeleton::sendSkinBones(const mat4* bones, const int* boneList, int boneCount) const
{
	static mat4 buffer[MAX_SHADER_BONES];

	if (!boneList)
	{
		boneCount = m_skinBoneCount;

		for (int i = 0; i < boneCount; i++)
			buffer[i] = bones[i] * m_bones[i].inverseTM;
	}

	gl::uniform(Shaders::skin.uBones, boneCount, buffer);
}