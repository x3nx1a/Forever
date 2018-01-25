#pragma once

enum VertexAttribute
{
	VATTRIB_POS = 0,
	VATTRIB_TEXCOORD0,
	VATTRIB_TEXCOORD1,
	VATTRIB_DIFFUSE,
	VATTRIB_NORMAL,
	VATTRIB_WEIGHT,
	VATTRIB_BONEID
};

struct TerrainVertex
{
	vec3 p;
	vec3 n;
	float tu1, tv1;
	float tu2, tv2;
};

struct WaterVertex
{
	vec3 p;
	vec2 t;
	u8vec4 d;
};

struct CloudVertex
{
	vec3 p;
	vec2 t;
};

struct SkyboxVertex
{
	vec3 p;
	float tu1, tv1;
	float a;
};

struct NormalObjectVertex
{
	vec3 p;
	vec3 n;
	vec2 t;
};

struct SkinObjectVertex
{
	vec3 p;
	float w1, w2;
	uint16_t id1, id2;
	vec3 n;
	vec2 t;
};

struct SfxVertex
{
	vec3 p;
	vec2 t;
};

struct RainVertex
{
	vec3 p;
	float a;
};

struct SnowVertex
{
	vec3 p;
};

struct Vertex2D
{
	vec2 p;
	vec2 t;
	u8vec4 c;
};