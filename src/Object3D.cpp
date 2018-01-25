#include "StdAfx.hpp"
#include "Object3D.hpp"
#include "Shaders.hpp"
#include "ShaderVars.hpp"
#include "TextureManager.hpp"
#include "GameTime.hpp"

Object3D::Object3D()
	: m_hasCollObj(false),
	m_LOD(false),
	m_collVertexCount(0),
	m_collVertices(nullptr),
	m_collIndexCount(0),
	m_collIndices(nullptr),
	m_vertexBufferSize(0),
	m_vertexBufferData(nullptr),
	m_indices(nullptr),
	m_indexCount(0),
	m_textureCount(0),
	m_textureNames(nullptr),
	m_materialBlockCount(0),
	m_materialBlocks(nullptr),
	m_geometryObjectCount(0),
	m_geometryObjects(nullptr),
	m_normalVertexCount(0),
	m_skinVertexCount(0),
	m_textures(nullptr)
{
}

Object3D::~Object3D()
{
	if (gl::isContextActive())
		onContextLost();

	if (m_collVertices)
		delete[] m_collVertices;
	if (m_collIndices)
		delete[] m_collIndices;
	if (m_vertexBufferData)
		delete[] m_vertexBufferData;
	if (m_indices)
		delete[] m_indices;
	if (m_textureNames)
		delete[] m_textureNames;
	if (m_materialBlocks)
		delete[] m_materialBlocks;
	if (m_geometryObjects)
		delete[] m_geometryObjects;
	if (m_textures)
		delete[] m_textures;
}

void Object3D::onContextLost()
{
	m_normalVAO.destroy();
	m_skinVAO.destroy();
	m_VBO.destroy();
	m_IBO.destroy();
}

void Object3D::onContextRestored()
{
	m_VBO.create();
	m_IBO.create();

	if (m_normalVertexCount)
	{
		m_normalVAO.create();
		m_normalVAO.bind();
		m_IBO.bind(m_normalVAO);
	}

	if (m_skinVertexCount)
	{
		m_skinVAO.create();
		m_skinVAO.bind();
		m_IBO.bind(m_skinVAO);
	}

	m_VBO.bind();
	m_VBO.data(m_vertexBufferSize, m_vertexBufferData);
	m_IBO.data(m_indexCount * sizeof(uint16_t), m_indices);

	if (m_normalVertexCount)
	{
		m_normalVAO.bind();
		m_normalVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(NormalObjectVertex), 0);
		m_normalVAO.vertexAttribPointer<vec3>(VATTRIB_NORMAL, false, sizeof(NormalObjectVertex), sizeof(vec3));
		m_normalVAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(NormalObjectVertex), sizeof(vec3) * 2);
	}

	if (m_skinVertexCount)
	{
		m_skinVAO.bind();

		const uint32_t baseVertexPtr = m_normalVertexCount * sizeof(NormalObjectVertex);
		m_skinVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(SkinObjectVertex), baseVertexPtr);
		m_skinVAO.vertexAttribPointer<vec2>(VATTRIB_WEIGHT, false, sizeof(SkinObjectVertex), baseVertexPtr + sizeof(vec3));
		m_skinVAO.vertexAttribPointer<u16vec2>(VATTRIB_BONEID, false, sizeof(SkinObjectVertex), baseVertexPtr + sizeof(vec3) + sizeof(vec2));
		m_skinVAO.vertexAttribPointer<vec3>(VATTRIB_NORMAL, false, sizeof(SkinObjectVertex), baseVertexPtr + sizeof(vec3) + sizeof(vec2) + sizeof(uint32_t));
		m_skinVAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(SkinObjectVertex), baseVertexPtr + sizeof(vec3) + sizeof(vec2) + sizeof(uint32_t) + sizeof(vec3));
	}
}

void Object3D::render(const mat4* bones, const mat4& world, int lod, int textureEx, uint32_t effect, float alpha) const
{
	TexturePtr* const textures = &m_textures[m_textureCount * textureEx];
	const LODGroup& group = m_groups[m_LOD ? lod : 0];

	if (alpha < 0.999f)
		effect |= Opacity;

	GeometryObjectType curType = GMT_ERROR;
	Shaders::ObjectProgram* program = nullptr;
	ivec3 shaderEffects;
	mat4 worldTM;

	for (int i = 0; i < group.objectCount; i++)
	{
		const GeometryObject& obj = group.objects[i];

		if (obj.type != curType)
		{
			curType = obj.type;

			if (curType == GMT_LIGHT && (!GameTime::isNight() || (effect & NoEffect)))
				break;

			if (curType != GMT_SKIN)
			{
				m_normalVAO.bind();
				program = &Shaders::object;
			}
			else
			{
				m_skinVAO.bind();
				program = &Shaders::skin;
			}

			if (curType != GMT_LIGHT)
				gl::enableDepthWrite();
			else
			{
				gl::disableDepthWrite();
				gl::disableCull();
				gl::enableBlend();
				gl::blendFunc(GL_ONE, GL_ONE);

				shaderEffects = ivec3(false);
			}

			program->use();
		}

		if (curType == GMT_SKIN)
			worldTM = world;
		else
			worldTM = world * bones[obj.boneId];

		const mat4 WVP = ShaderVars::viewProj * worldTM;

		gl::uniform(program->uWVP, WVP);
		gl::uniform(program->uWorld, worldTM);

		for (int j = 0; j < obj.materialBlockCount; j++)
		{
			const MaterialBlock& block = obj.materialBlocks[j];

			if (curType != GMT_LIGHT)
			{
				if (block.effect & Opacity)
				{
					gl::enableBlend();
					if (!(effect & NoEffect))
						gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}
				else
					gl::disableBlend();

				if (block.effect & TwoSides)
					gl::disableCull();
				else
					gl::enableCull();

				if (effect & NoEffect)
					shaderEffects = ivec3(false);
				else
					shaderEffects = ivec3(true, !(block.effect & SelfIlluminate), (block.effect & Reflect));
			}

			if (block.textureId == -1)
				ShaderVars::blankTexture.bind();
			else
				textures[block.textureId]->bind();

			if (program->curEffects != shaderEffects)
			{
				gl::uniform(program->uEffects, shaderEffects);
				program->curEffects = shaderEffects;
			}

			gl::drawElements<uint16_t>(GL_TRIANGLES, block.indexCount, block.indices);
		}
	}
}

void Object3D::load(BinaryReader& reader, uint8_t ver)
{
	reader >> m_bbMin
		>> m_bbMax
		>> m_LOD
		>> m_hasCollObj;

	reader >> m_collVertexCount;
	if (m_collVertexCount)
	{
		m_collVertices = new vec3[m_collVertexCount];
		reader.read(m_collVertices, m_collVertexCount);
	}

	reader >> m_collIndexCount;
	if (m_collIndexCount)
	{
		m_collIndices = new uint16_t[m_collIndexCount];
		reader.read(m_collIndices, m_collIndexCount);
	}

	reader >> m_normalVertexCount
		>> m_skinVertexCount;

	m_vertexBufferSize = sizeof(NormalObjectVertex) * m_normalVertexCount + sizeof(SkinObjectVertex) * m_skinVertexCount;
	m_vertexBufferData = new char[m_vertexBufferSize];
	reader.read(m_vertexBufferData, m_vertexBufferSize);

	reader >> m_indexCount;
	if (m_indexCount)
	{
		m_indices = new uint16_t[m_indexCount];
		reader.read(m_indices, m_indexCount);
	}

	reader >> m_textureCount;
	if (m_textureCount)
	{
		m_textureNames = new char[m_textureCount * 128];

		int bufferLen;
		for (int i = 0; i < m_textureCount; i++)
		{
			reader >> bufferLen;
			reader.read(&m_textureNames[i * 128], bufferLen);
			m_textureNames[i * 128 + bufferLen] = '\0';
		}
	}

	m_textures = new TexturePtr[m_textureCount * MAX_TEXTURE_EX];

	reader >> m_materialBlockCount;
	if (m_materialBlockCount)
	{
		m_materialBlocks = new MaterialBlock[m_materialBlockCount];
		for (int i = 0; i < m_materialBlockCount; i++)
		{
			MaterialBlock& block = m_materialBlocks[i];

			reader >> block.indexCount
				>> block.textureId
				>> block.effect
				>> block.alpha
				>> block.indices;
		}
	}

	reader >> m_geometryObjectCount;
	if (m_geometryObjectCount)
	{
		m_geometryObjects = new GeometryObject[m_geometryObjectCount];
		for (int i = 0; i < m_geometryObjectCount; i++)
		{
			GeometryObject& gm = m_geometryObjects[i];

			reader >> gm.type;

			gm.indices = &m_indices[reader.read<int>()];
			reader >> gm.triangleCount;

			gm.materialBlocks = &m_materialBlocks[reader.read<int>()];
			reader >> gm.materialBlockCount;

			reader >> gm.boneId;
		}
	}

	for (int i = 0; i < (m_LOD ? LOD_COUNT : 1); i++)
	{
		LODGroup& group = m_groups[i];

		group.objects = &m_geometryObjects[reader.read<int>()];
		reader >> group.objectCount;
	}

	if (gl::isContextActive())
		onContextRestored();
}

void Object3D::loadTextureEx(int textureEx)
{
	TexturePtr* const textures = &m_textures[m_textureCount * textureEx];

	for (int i = 0; i < m_textureCount; i++)
	{
		if (!textures[i])
			textures[i] = TextureManager::getModelTexture(&m_textureNames[i * 128]);
	}
}