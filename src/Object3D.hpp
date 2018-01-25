#pragma once

#include "Texture.hpp"

#define LOD_COUNT 3
#define MAX_TEXTURE_EX 8

struct MaterialBlock
{
	int indexCount;
	int textureId;
	uint32_t effect;
	float alpha;
	uint32_t indices;
};

enum GeometryObjectType
{
	GMT_ERROR = -1,
	GMT_NORMAL,
	GMT_SKIN,
	GMT_BONE,
	GMT_LIGHT
};

struct GeometryObject
{
	GeometryObjectType type;
	uint16_t* indices;
	int triangleCount;
	MaterialBlock* materialBlocks;
	int materialBlockCount;
	int boneId;
};

struct LODGroup
{
	GeometryObject* objects;
	int objectCount;
};

class Object3D : public gl::DeviceObject
{
public:
	enum Effect
	{
		NoEffect = 1 << 0,
		Reflect = 1 << 1,
		TwoSides = 1 << 2,
		SelfIlluminate = 1 << 3,
		HighlightObject = 1 << 4,
		Opacity = 1 << 5
	};

public:
	explicit Object3D();
	virtual ~Object3D();

	void load(BinaryReader& reader, uint8_t ver);
	void loadTextureEx(int textureEx);

	void render(const mat4* bones, const mat4& world, int lod, int textureEx, uint32_t effect, float alpha) const;

	void bounds(vec3& bbMin, vec3& bbMax) const {
		bbMin = m_bbMin;
		bbMax = m_bbMax;
	}

protected:
	virtual void onContextLost();
	virtual void onContextRestored();

private:
	vec3 m_bbMin, m_bbMax;
	bool m_LOD, m_hasCollObj;
	int m_collVertexCount;
	vec3* m_collVertices;
	int m_collIndexCount;
	uint16_t* m_collIndices;
	int m_normalVertexCount;
	int m_skinVertexCount;
	int m_vertexBufferSize;
	char* m_vertexBufferData;
	uint16_t* m_indices;
	int m_indexCount;
	int m_textureCount;
	char* m_textureNames;
	int m_materialBlockCount;
	MaterialBlock* m_materialBlocks;
	int m_geometryObjectCount;
	GeometryObject* m_geometryObjects;
	LODGroup m_groups[LOD_COUNT];
	gl::VertexBuffer m_VBO;
	gl::IndexBuffer m_IBO;
	gl::VertexArray m_normalVAO, m_skinVAO;
	TexturePtr* m_textures;
};