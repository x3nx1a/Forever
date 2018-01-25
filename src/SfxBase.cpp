#include "StdAfx.hpp"
#include "SfxBase.hpp"
#include "ModelManager.hpp"
#include "Object3D.hpp"
#include "GeometryUtils.hpp"
#include "TextureManager.hpp"
#include "Shaders.hpp"

#define MAX_POINTS_SFX_CUSTOMMESH 180

SfxPart::SfxPart()
	: m_keys(nullptr),
	m_keyCount(0),
	m_texFrame(1),
	m_texLoop(1),
	m_billType(SfxPartBillType::Bill),
	m_alphaType(SfxPartAlphaType::Blend),
	m_textureName(""),
	m_textures(nullptr),
	m_hasTexture(false)
{
}

SfxPart::~SfxPart()
{
	if (m_keys)
		delete[] m_keys;
	if (m_textures)
		delete[] m_textures;
}

void SfxPart::load(BinaryReader& reader, uint8_t ver)
{
	reader >> m_billType
		>> m_alphaType
		>> m_texFrame
		>> m_texLoop;

	const uint8_t textureNameLen = reader.read<uint8_t>();
	reader.read(m_textureName, textureNameLen);
	m_textureName[textureNameLen] = '\0';

	reader >> m_keyCount;

	m_keys = new SfxKeyFrame[m_keyCount];
	for (int i = 0; i < m_keyCount; i++)
	{
		SfxKeyFrame& key = m_keys[i];

		reader >> key.pos
			>> key.posRotate
			>> key.rotate
			>> key.scale
			>> key.alpha
			>> key.frame;
	}

	setTexture();
}

void SfxPart::setTexture()
{
	if (m_textures)
	{
		delete[] m_textures;
		m_textures = nullptr;
	}

	if (strlen(m_textureName) == 0)
	{
		m_hasTexture = false;
		return;
	}

	m_hasTexture = true;

	if (m_texFrame > 1)
	{
		int numDigits = 0;
		int strIndex = strlen(m_textureName) - 1;

		while (isdigit(m_textureName[strIndex]))
		{
			numDigits++;
			if (!strIndex)
				break;
			strIndex--;
		}

		const int nameLen = strlen(m_textureName) - numDigits;
		char name[128];
		strncpy(name, m_textureName, nameLen);
		name[nameLen] = '\0';

		char textureName[128];
		m_textures = new TexturePtr[m_texFrame];

		if (numDigits == 1)
		{
			for (int i = 0; i < (int)m_texFrame; i++)
			{
				sprintf(textureName, "%s%d", name, i + 1);
				m_textures[i] = TextureManager::getSfxTexture(textureName);
			}
		}
		else
		{
			const int numStart = stol(m_textureName + nameLen);

			for (int i = 0; i < (int)m_texFrame; i++)
			{
				sprintf(textureName, "%s%02d", name, numStart + i);
				m_textures[i] = TextureManager::getSfxTexture(textureName);
			}
		}
	}
	else
		m_texture = TextureManager::getSfxTexture(m_textureName);
}

const SfxKeyFrame* SfxPart::getKey(int frame) const
{
	for (int i = 0; i < m_keyCount; i++)
		if (m_keys[i].frame == frame)
			return &m_keys[i];
	return nullptr;
}

const bool SfxPart::getKey(int frame, SfxKeyFrame& key) const
{
	const SfxKeyFrame* prevKey = getPrevKey(frame);
	const SfxKeyFrame* nextKey = getNextKey(frame);
	if (!prevKey || !nextKey)
		return false;

	const int deltaFrame = nextKey->frame - prevKey->frame;
	key = *prevKey;
	if (deltaFrame != 0)
	{
		key.pos += (nextKey->pos - prevKey->pos) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
		key.posRotate += (nextKey->posRotate - prevKey->posRotate) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
		key.rotate += (nextKey->rotate - prevKey->rotate) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
		key.scale += (nextKey->scale - prevKey->scale) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
		key.alpha += (nextKey->alpha - prevKey->alpha) * (frame - prevKey->frame) / deltaFrame;
		key.frame = deltaFrame;
	}

	return true;
}

const SfxKeyFrame* SfxPart::getFirstKey() const
{
	if (m_keyCount > 0)
		return &m_keys[0];
	return nullptr;
}

const SfxKeyFrame* SfxPart::getLastKey() const
{
	if (m_keyCount > 0)
		return &m_keys[m_keyCount - 1];
	return nullptr;
}

const SfxKeyFrame* SfxPart::getPrevKey(int frame) const
{
	const SfxKeyFrame* key = nullptr;
	for (int i = 0; i < m_keyCount; i++)
	{
		if (m_keys[i].frame > frame)
			break;
		key = &m_keys[i];
	}
	return key;
}

const SfxKeyFrame* SfxPart::getNextKey(int frame, bool skip) const
{
	const SfxKeyFrame* key = nullptr;
	if (skip)
	{
		for (int i = 0; i < m_keyCount; i++)
		{
			if (m_keys[i].frame >= frame)
			{
				key = &m_keys[i];
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < m_keyCount; i++)
		{
			if (m_keys[i].frame > frame)
			{
				key = &m_keys[i];
				break;
			}
		}

	}
	return key;
}

SfxPartBill::SfxPartBill()
{
}

void SfxPartBill::render(int frame, const vec3& pos, const vec3& angle, const vec3& scale)
{
	SfxKeyFrame key;
	if (!getKey(frame, key))
		return;

	ShaderVars::sfxVAO.bind();

	if (m_hasTexture)
	{
		if (m_texFrame > 1)
		{
			const SfxKeyFrame* firstKey = getNextKey(0);
			if (!firstKey || frame < firstKey->frame)
				return;
			m_textures[(int)((m_texFrame * (frame - firstKey->frame) / m_texLoop) % m_texFrame)]->bind();
		}
		else
			m_texture->bind();
	}
	else
		ShaderVars::blankTexture.bind();

	const mat4 matScale = glm::scale(mat4(), scale * key.scale);
	const vec3 rot = radians(angle);
	const mat4 matRot = yawPitchRoll(rot.y, rot.x, rot.z);

	mat4 matTemp(uninitialize);

	switch (m_billType)
	{
	case SfxPartBillType::Pole:
	case SfxPartBillType::Bill:
		matTemp = ShaderVars::invView * rotate(mat4(), radians(key.rotate.z), vec3(0, 0, 1)) * matScale;
		break;
	case SfxPartBillType::Bottom:
		matTemp = rotate(mat4(), radians(90.0f), vec3(1, 0, 0)) * rotate(mat4(), radians(key.rotate.z - angle.z), vec3(0, 0, 1)) * matScale;
		break;
	case SfxPartBillType::Normal:
	{
		const vec3 temp = radians(key.rotate);
		matTemp = matRot * yawPitchRoll(temp.y, temp.x, temp.z) * matScale;
		break;
	}
	}

	const vec3 temp = radians(key.posRotate);
	const mat4 world = ShaderVars::viewProj * (translate(mat4(), pos + transformCoord(key.pos, yawPitchRoll(temp.y, temp.x, temp.z)) * scale) * matTemp);

	if (m_alphaType == SfxPartAlphaType::Glow)
		gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
	else
		gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl::uniform(Shaders::sfx.uWVP, world);

	if (Shaders::sfx.alphaFactor != key.alpha)
	{
		gl::uniform(Shaders::sfx.uAlphaFactor, key.alpha);
		Shaders::sfx.alphaFactor = key.alpha;
	}

	gl::drawArrays(GL_TRIANGLE_FAN, 0, 4);
}

SfxPartMesh::SfxPartMesh()
	: m_obj3d(nullptr),
	m_bones(nullptr)
{
}

SfxPartMesh::~SfxPartMesh()
{
	if (m_bones)
		delete[] m_bones;
}

void SfxPartMesh::render(int frame, const vec3& pos, const vec3& angle, const vec3& scale)
{
	if (!m_obj3d)
	{
		if (!m_modelFile->loaded())
			return;

		m_obj3d = m_modelFile->object();
		m_obj3d->loadTextureEx(0);

		m_bones = m_modelFile->skeleton()->createBones();
	}

	SfxKeyFrame key;
	if (!getKey(frame, key))
		return;

	const vec3 rot = radians(key.rotate + vec3(0.0f, angle.y, 0.0f));
	const vec3 posRot = radians(key.posRotate + vec3(0.0f, angle.y, 0.0f));

	const mat4 world = translate(mat4(), pos + transformCoord(key.pos, yawPitchRoll(posRot.y, posRot.x, posRot.z)) * scale)
		* yawPitchRoll(rot.y, rot.x, rot.z)
		* glm::scale(mat4(), scale * key.scale);

	if (m_alphaType == SfxPartAlphaType::Glow)
		gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
	else
		gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_obj3d->render(m_bones, world, 0, 0, Object3D::NoEffect, key.alpha);
}

void SfxPartMesh::setTexture()
{
	m_modelFile = ModelManager::getModelFile(m_textureName);
	m_obj3d = nullptr;
}

SfxPartCustomMesh::SfxPartCustomMesh()
	: m_pointCount(0)
{
}

void SfxPartCustomMesh::render(int frame, const vec3& pos, const vec3& angle, const vec3& scale)
{
	SfxKeyFrame key;
	if (!getKey(frame, key))
		return;

	ShaderVars::customSfxVAO.bind();

	if (m_hasTexture)
	{
		if (m_texFrame > 1)
		{
			const SfxKeyFrame* firstKey = getNextKey(0);
			if (!firstKey || frame < firstKey->frame)
				return;
			m_textures[(int)((m_texFrame * (frame - firstKey->frame) / m_texLoop) % m_texFrame)]->bind();
		}
		else
			m_texture->bind();
	}
	else
		ShaderVars::blankTexture.bind();

	const vec3 rot = radians(angle);
	const mat4 matRot = yawPitchRoll(rot.y, rot.x, rot.z);

	mat4 matTemp(uninitialize);

	switch (m_billType)
	{
	case SfxPartBillType::Bill:
		matTemp = ShaderVars::invView * rotate(mat4(), radians(key.rotate.z - angle.z), vec3(0, 0, 1));
		break;
	case SfxPartBillType::Bottom:
		matTemp = rotate(mat4(), radians(90.0f), vec3(1, 0, 0)) * rotate(mat4(), radians(key.rotate.z), vec3(0, 0, 1));
		break;
	case SfxPartBillType::Pole:
		matTemp = ShaderVars::invView * rotate(mat4(), radians(key.rotate.z), vec3(0, 0, 1));
		break;
	case SfxPartBillType::Normal:
	{
		const vec3 temp = radians(key.rotate);
		matTemp = matRot * yawPitchRoll(temp.y, temp.x, temp.z);
		break;
	}
	}

	const mat4 world = ShaderVars::viewProj * (translate(mat4(), pos + transformCoord(key.pos, matRot) * scale) * matTemp);

	static SfxVertex vertices[((MAX_POINTS_SFX_CUSTOMMESH * 2) + 1) * 2];

	float texX = 0.0f;

	for (int i = 0; i < (m_pointCount * 2) + 1; i++)
	{
		const float tmpAnglePos = radians((float)(360 / (m_pointCount * 2) * i));
		const float sinAnglePos = sin(tmpAnglePos);
		const float cosAnglePos = cos(tmpAnglePos);

		vertices[i * 2].p = vec3(sinAnglePos * key.scale.x, key.scale.y, cosAnglePos * key.scale.x) * scale;
		vertices[i * 2 + 1].p = vec3(sinAnglePos * key.scale.z, 0.0f, cosAnglePos * key.scale.z) * scale;

		vertices[i * 2].t.y = 1.0f - (key.posRotate.y + 0.1f);
		vertices[i * 2 + 1].t.y = 1.0f - key.posRotate.z;

		if (key.posRotate.x == 0.0f)
		{
			if (i / 2 * 2 == i)
				texX = 0.0f;
			else
				texX = 1.0f;
		}
		else
			texX = key.posRotate.x * ((float)i) / (m_pointCount * 2);

		vertices[i * 2].t.x = vertices[i * 2 + 1].t.x = texX;
	}

	ShaderVars::customSfxVBO.bind();
	ShaderVars::customSfxVBO.data(((m_pointCount * 2) + 1) * 2 * sizeof(SfxVertex), &vertices, true);

	if (m_alphaType == SfxPartAlphaType::Glow)
		gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
	else
		gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl::uniform(Shaders::sfx.uWVP, world);

	if (Shaders::sfx.alphaFactor != key.alpha)
	{
		gl::uniform(Shaders::sfx.uAlphaFactor, key.alpha);
		Shaders::sfx.alphaFactor = key.alpha;
	}

	gl::frontFace(GL_CW);
	gl::drawArrays(GL_TRIANGLE_STRIP, 0, m_pointCount * 4 + 2);
	gl::frontFace(GL_CCW);
	gl::drawArrays(GL_TRIANGLE_STRIP, 0, m_pointCount * 4 + 2);
}

void SfxPartCustomMesh::load(BinaryReader& reader, uint8_t ver)
{
	SfxPart::load(reader, ver);

	reader >> m_pointCount;
}

SfxPartParticle::SfxPartParticle()
	: m_particleCreate(0),
	m_particleCreateNum(5),
	m_particleFrameAppear(0),
	m_particleFrameKeep(10),
	m_particleFrameDisappear(20),
	m_particleStartPosVar(0.0f),
	m_particleStartPosVarY(0.0f),
	m_particleYLow(0.01f),
	m_particleYHigh(0.1f),
	m_particleXZLow(0.01f),
	m_particleXZHigh(0.1f),
	m_particleAccel(0.0f, 0.0f, 0.0f),
	m_scale(0.1f, 0.1f, 0.1f),
	m_scaleSpeed(0.0f, 0.0f, 0.0f),
	m_rotation(0.0f, 0.0f, 0.0f),
	m_rotationLow(0.0f, 0.0f, 0.0f),
	m_rotationHigh(0.0f, 0.0f, 0.0f),
	m_repeatScal(false),
	m_repeat(false),
	m_scalSpeedXLow(0),
	m_scalSpeedXHigh(0),
	m_scalSpeedYLow(0),
	m_scalSpeedYHigh(0),
	m_scalSpeedZLow(0),
	m_scalSpeedZHigh(0),
	m_scaleSpeed2(0.0f, 0.0f, 0.0f),
	m_scaleEnd(0.0f, 0.0f, 0.0f)
{
}

void SfxPartParticle::load(BinaryReader& reader, uint8_t ver)
{
	SfxPart::load(reader, ver);

	reader >> m_particleCreate
		>> m_particleCreateNum
		>> m_particleFrameAppear
		>> m_particleFrameKeep
		>> m_particleFrameDisappear
		>> m_particleStartPosVar
		>> m_particleStartPosVarY
		>> m_particleYLow
		>> m_particleYHigh
		>> m_particleXZLow
		>> m_particleXZHigh
		>> m_particleAccel
		>> m_scale
		>> m_scaleSpeed
		>> m_rotation
		>> m_rotationLow
		>> m_rotationHigh
		>> m_repeatScal
		>> m_scalSpeedXLow
		>> m_scalSpeedXHigh
		>> m_scalSpeedYLow
		>> m_scalSpeedYHigh
		>> m_scalSpeedZLow
		>> m_scalSpeedZHigh
		>> m_scaleSpeed2
		>> m_scaleEnd
		>> m_repeat;
}

void SfxPartParticle::render(int frame, const vec3& pos, const vec3& angle, const vec3& scale)
{
	// In the SfxModel
}

SfxBase::SfxBase()
	: m_partCount(0),
	m_parts(nullptr),
	m_frameCount(0)
{
}

SfxBase::~SfxBase()
{
	if (m_parts)
	{
		for (int i = 0; i < m_partCount; i++)
			delete m_parts[i];
		delete[] m_parts;
	}
}

void SfxBase::load(BinaryReader& reader, uint8_t ver)
{
	reader >> m_bbMin
		>> m_bbMax
		>> m_frameCount
		>> m_partCount;

	m_parts = new SfxPart*[m_partCount];

	SfxPart* part;
	for (int i = 0; i < m_partCount; i++)
	{
		switch (reader.read<SfxPartType>())
		{
		case SfxPartType::Bill:
			part = new SfxPartBill();
			break;
		case SfxPartType::Particle:
			part = new SfxPartParticle();
			break;
		case SfxPartType::Mesh:
			part = new SfxPartMesh();
			break;
		case SfxPartType::CustomMesh:
			part = new SfxPartCustomMesh();
			break;
		default:
			part = nullptr;
			break;
		}

		if (part)
			part->load(reader, ver);

		m_parts[i] = part;
	}
}