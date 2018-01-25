#include "StdAfx.hpp"
#include "SfxModel.hpp"
#include "ModelManager.hpp"
#include "SfxBase.hpp"
#include "Shaders.hpp"
#include "GeometryUtils.hpp"

SfxModel::SfxModel(const ModelProp* prop)
	: Model(prop),
	m_partCount(0),
	m_parts(nullptr),
	m_particles(nullptr)
{
}

SfxModel::~SfxModel()
{
	if (m_particles)
	{
		for (int i = 0; i < m_partCount; i++)
			if (m_particles[i])
				delete m_particles[i];
		delete[] m_particles;
	}
}

void SfxModel::render(const vec3& pos, const vec3& angle, const vec3& scale) const
{
	const int frame = (int)m_currentFrame;

	bool vboBind = false, programBind = false;

	for (int i = 0; i < m_partCount; i++)
	{
		SfxPart* const part = m_parts[i];

		const SfxPartType type = part->type();

		if (type != SfxPartType::Mesh && !programBind)
		{
			Shaders::sfx.use();
			gl::disableDepthWrite();
			gl::disableCull();
			gl::enableBlend();

			programBind = true;
		}

		switch (type)
		{
		case SfxPartType::Bill:
			part->render(frame, pos, angle, scale);
			break;
		case SfxPartType::CustomMesh:
			part->render(frame, pos, angle, scale);
			break;
		case SfxPartType::Particle:
			renderParticles(*m_particles[i], (const SfxPartParticle*)part, pos, angle, scale);
			break;
		case SfxPartType::Mesh:
			part->render(frame, pos, angle, scale);
			programBind = false;
			break;
		}
	}
}

void SfxModel::renderParticles(const SfxParticle::Array& particles, const SfxPartParticle* part, const vec3& pos, const vec3& angle, const vec3& scale) const
{
	const int frame = (int)m_currentFrame;

	SfxKeyFrame key;
	if (!part->getKey(frame, key))
		return;

	ShaderVars::sfxVAO.bind();

	const int prevFrame = part->getPrevKey(frame)->frame;

	const vec3 rot = radians(angle);
	const mat4 matRot = yawPitchRoll(rot.y, rot.x, rot.z);

	mat4 matTemp(uninitialize);

	switch (part->m_billType)
	{
	case SfxPartBillType::Pole:
	case SfxPartBillType::Bill:
		matTemp = ShaderVars::invView * rotate(mat4(), radians(key.rotate.z), vec3(0, 0, 1));
		break;
	case SfxPartBillType::Bottom:
		matTemp = rotate(mat4(), radians(90.0f), vec3(1, 0, 0)) * rotate(mat4(), radians(key.rotate.z), vec3(0, 0, 1));
		break;
	case SfxPartBillType::Normal:
	{
		const vec3 keyRot = radians(key.rotate);
		matTemp = matRot * yawPitchRoll(keyRot.y, keyRot.x, keyRot.z);
		break;
	}
	}

	if (part->m_alphaType == SfxPartAlphaType::Glow)
		gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
	else
		gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (part->m_hasTexture)
	{
		if (part->m_texFrame <= 1)
			part->m_texture->bind();
	}
	else
		ShaderVars::blankTexture.bind();

	int sendTexFrame = -1;
	vec3 tempRot;
	float tempAlpha;

	const vec3 keyPosRotate = radians(key.posRotate);
	const vec3 temp = transformCoord(key.pos, yawPitchRoll(keyPosRotate.y, keyPosRotate.x, keyPosRotate.z) * matRot);
	const vec3 temp2 = radians(key.rotate + key.posRotate);
	const mat4 matTemp2 = matRot * yawPitchRoll(temp2.y, temp2.x, temp2.z);

	for (auto it = particles.begin(); it != particles.end(); it++)
	{
		const SfxParticle& particle = *it;

		if (part->m_hasTexture && part->m_texFrame > 1)
		{
			const int texFrame = (part->m_texFrame * particle.frame / part->m_texLoop) % part->m_texFrame;
			if (texFrame != sendTexFrame)
			{
				part->m_textures[texFrame]->bind();
				sendTexFrame = texFrame;
			}
		}

		const mat4 matTrans = translate(mat4(), temp + (transformCoord(particle.pos, matTemp2) * scale) + pos);
		const mat4 matScale = glm::scale(mat4(), scale * vec3(particle.scale.x, particle.scale.y, 1.0f));

		if (key.frame != 0)
			tempRot = radians(glm::lerp(vec3(0.0f, 0.0f, 0.0f), particle.rotation, (float)(frame - prevFrame) / key.frame));
		else
			tempRot = vec3(0.0f, 0.0f, 0.0f);

		const mat4 world = ShaderVars::viewProj * (matTrans * matTemp * matScale * yawPitchRoll(tempRot.y, tempRot.x, tempRot.z));

		gl::uniform(Shaders::sfx.uWVP, world);

		if (particle.frame < part->m_particleFrameAppear)
			tempAlpha = particle.frame * key.alpha / part->m_particleFrameAppear;
		else if (particle.frame > part->m_particleFrameKeep)
			tempAlpha = key.alpha - (key.alpha * (particle.frame - part->m_particleFrameKeep) / (part->m_particleFrameDisappear - part->m_particleFrameKeep));
		else
			tempAlpha = key.alpha;

		if (Shaders::sfx.alphaFactor != tempAlpha)
		{
			gl::uniform(Shaders::sfx.uAlphaFactor, tempAlpha);
			Shaders::sfx.alphaFactor = tempAlpha;
		}

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
}

void SfxModel::update(int frameCount)
{
	if (!m_loaded || !frameCount)
		return;

	m_endFrame = ((int)m_currentFrame + frameCount >= m_frameCount);
	const int frameRangeBegin = m_endFrame ? 0 : (int)m_currentFrame;
	m_currentFrame = mod(m_currentFrame + frameCount, (float)m_frameCount);
	const int frameRangeEnd = (int)m_currentFrame;

	for (int i = 0; i < m_partCount; i++)
	{
		if (m_particles[i])
		{
			const SfxPartParticle* const part = (SfxPartParticle*)m_parts[i];
			SfxParticle::Array& particles = *m_particles[i];

			if (m_endFrame)
				particles.clear();

			const int startFrame = part->getFirstKey()->frame;
			const int endFrame = part->getLastKey()->frame;

			for (int frame = frameRangeBegin; frame < frameRangeEnd; frame++)
			{
				for (auto it = particles.begin(); it != particles.end(); )
				{
					SfxParticle& particle = *it;

					particle.frame++;
					if (particle.frame >= part->m_particleFrameDisappear)
						it = particles.erase(it);
					else
					{
						particle.pos += particle.speed;
						particle.speed += part->m_particleAccel;

						if (part->m_repeatScal)
						{
							const float lenScale = length(particle.scale);

							if (lenScale >= length(part->m_scaleEnd))
								particle.scal = true;
							else if (lenScale <= length(part->m_scale))
								particle.scal = false;

							if (particle.scal)
								particle.scale -= particle.scaleSpeed;
							else
								particle.scale += particle.scaleSpeed;
						}
						else
							particle.scale += part->m_scaleSpeed;

						it++;
					}
				}

				if (frame >= startFrame && frame <= endFrame - part->m_particleFrameDisappear)
				{
					if ((part->m_particleCreate == 0 && frame == startFrame) ||
						(part->m_particleCreate != 0 && (frame - startFrame) % part->m_particleCreate == 0))
					{
						const float rand1 = (rand() % 50000) / 50000.0f;
						const float rand2 = (rand() % 50000) / 50000.0f;
						const float rand3 = (rand() % 50000) / 50000.0f;

						for (int j = 0; j < part->m_particleCreateNum; j++)
						{
							SfxParticle particle;
							particle.frame = 0;
							particle.scale = part->m_scale;
							particle.scal = false;

							const float angle = ((rand() % 50000) / 50000.0f) * 360.0f;
							const float factor = part->m_particleXZLow + rand2 * (part->m_particleXZHigh - part->m_particleXZLow);

							particle.pos = vec3(sin(angle) * part->m_particleStartPosVar,
								rand1 * part->m_particleStartPosVarY,
								cos(angle) * part->m_particleStartPosVar);
							particle.speed = vec3(sin(angle) * factor,
								part->m_particleYLow + rand2 * (part->m_particleYHigh - part->m_particleYLow),
								cos(angle) * factor);
							particle.rotation = vec3(part->m_rotationLow.x + rand1 *
								(part->m_rotationHigh.x - part->m_rotationLow.x),
								part->m_rotationLow.y + rand3 *
								(part->m_rotationHigh.y - part->m_rotationLow.y),
								part->m_rotationLow.z + rand2 *
								(part->m_rotationHigh.z - part->m_rotationLow.z));
							particle.scaleSpeed = vec3(part->m_scalSpeedXLow + rand3 *
								(part->m_scalSpeedXHigh - part->m_scalSpeedXLow),
								part->m_scalSpeedYLow + rand2 *
								(part->m_scalSpeedYHigh - part->m_scalSpeedYLow),
								part->m_scalSpeedZLow + rand1 *
								(part->m_scalSpeedZHigh - part->m_scalSpeedZLow));

							particles.push_back(particle);
						}
					}
				}
			}

			/*if (particles.size() > 0 || frame < startFrame)
			ret = true;

			if (part->m_repeat)
			{
			if (endFrame >= 0)
			{
			const float f = m_currentFrame / (float)endFrame;
			if (f >= 0.65f)
			m_currentFrame = (float)endFrame * 0.6f;
			}
			}*/
		}
	}
}

void SfxModel::load(const string& filename)
{
	m_file = ModelManager::getModelFile(filename);
}

bool SfxModel::checkLoaded()
{
	if (!m_file->loaded())
		return false;

	SfxBase* sfx = m_file->sfx();

	m_partCount = sfx->partCount();
	m_parts = sfx->parts();

	if (m_partCount)
	{
		m_particles = new SfxParticle::Array*[m_partCount];
		for (int i = 0; i < m_partCount; i++)
		{
			if (m_parts[i]->type() == SfxPartType::Particle)
				m_particles[i] = new SfxParticle::Array();
			else
				m_particles[i] = nullptr;
		}
	}

	sfx->bounds(m_bbMin, m_bbMax);
	m_frameCount = (int)sfx->frameCount();

	m_loaded = true;
	return true;
}