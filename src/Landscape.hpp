#pragma once

#include "Texture.hpp"
#include "Object.hpp"

#define NUM_PATCHES_PER_SIDE	8
#define PATCH_SIZE 8
#define MAP_SIZE	(NUM_PATCHES_PER_SIDE * PATCH_SIZE)
#define LIGHTMAP_SIZE ((PATCH_SIZE - 1) * NUM_PATCHES_PER_SIDE)

#define HGT_NOWALK 1000.0f
#define HGT_NOFLY  2000.0f
#define HGT_NOMOVE 3000.0f
#define HGT_DIE 4000.0f

class World;

struct WaterHeight
{
	enum Type
	{
		None,
		Cloud,
		Water
	};

	uint16_t type;
	uint16_t texture;
	float height;
};

class Landscape : public Resource, public gl::DeviceObject
{
public:
	explicit Landscape(const string& filename, World* world, const ivec2& pos);
	virtual ~Landscape();

	float getHeightMap(uint offset) const;
	float getHeight_fast(float x, float z) const;
	float getHeight(float x, float z) const;
	const WaterHeight* getWaterHeight(int x, int z) const;

	void render();
	void renderWater(WaterHeight::Type type);
	void updateCull();

	void addObjArray(Object* obj);
	bool insertObjLink(Object* obj);
	void removeObjArray(Object* obj);

	bool visible() const {
		return m_visible;
	}
	const vector<Object*>& objects(ObjectType type) const {
		return m_objects[type];
	}

protected:
	virtual void onContextLost();
	virtual void onContextRestored();
	virtual void onLoad(BinaryReader reader);

private:
	struct Patch
	{
		bool visible;
		uint32_t indexOffset;
		vec3 bounds[8];

		void init(const float* landHeight, const WaterHeight& waterHeight, const ivec2& landPos, const ivec2& pos);
		void cull();
	};

	struct Layer
	{
		TexturePtr texture;
		bool patchEnabled[NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE];
		vec2 lightMapOffset;
	};

private:
	void setVertices();
	void cull();
	void calculateBounds();

private:
	World* const m_world;
	const ivec2 m_pos;
	float m_heightMap[(MAP_SIZE + 1) * (MAP_SIZE + 1)];
	gl::VertexBuffer m_VBO;
	gl::VertexArray m_VAO, m_waterVAO, m_cloudVAO;
	bool m_visible;
	Patch m_patches[NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE];
	vec3 m_bounds[8];
	Layer* m_layers;
	int m_layerCount;
	gl::Texture2D m_lightMap;
	ivec2 m_lightMapSize;
	u8vec4* m_lightMapData;
	int m_waterVertexCount;
	int m_cloudVertexCount;
	uint32_t m_waterVertexOffset;
	uint32_t m_cloudVertexOffset;
	WaterHeight m_waterHeight[NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE];
	vector<Object*> m_objects[MAX_OBJTYPE];
};

typedef RefCountedPtr<Landscape> LandscapePtr;