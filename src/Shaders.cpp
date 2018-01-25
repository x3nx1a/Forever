#include "StdAfx.hpp"
#include "Shaders.hpp"

#define FRAGMENT_PRECISION "mediump"

namespace Shaders
{
	namespace
	{
		gl::FragmentShader s_terrainFragment, s_waterFragment, s_cloudFragment, s_skyboxFragment,
			s_objectFragment, s_sfxFragment, s_rainFragment, s_snowFragment, s_render2dFragment;

		gl::VertexShader s_terrainVertex, s_waterVertex, s_cloudVertex, s_skyboxVertex,
			s_objectVertex, s_skinVertex, s_sfxVertex, s_rainVertex, s_snowVertex, s_render2dVertex;
	}

	TerrainProgram terrain;
	WaterProgram water;
	CloudProgram cloud;
	SkyboxProgram skybox;
	ObjectProgram object;
	SkinProgram skin;
	SfxProgram sfx;
	RainProgram rain;
	SnowProgram snow;
	Render2DProgram render2d;

	void createTerrainProgram()
	{
		s_terrainVertex.setSource(
			"attribute vec3 aPos;" \
			"attribute vec2 aTexCoord0;" \
			"attribute vec2 aTexCoord1;" \
			"attribute vec3 aNormal;" \

			"varying vec2 vTexCoord0;" \
			"varying vec2 vTexCoord1;" \
			"varying float vFogFactor;" \
			"varying vec3 vLightColor;" \

			"uniform mat4 uWVP;" \
			"uniform vec3 uCameraPos;" \
			"uniform vec2 uFogSettings;" \
			"uniform vec3 uAmbient;" \
			"uniform vec3 uDiffuse;" \
			"uniform vec3 uLightDir;" \

			"void main(void) {" \
			"	gl_Position = uWVP * vec4(aPos, 1.0);" \
			"	vTexCoord0 = aTexCoord0;" \
			"	vTexCoord1 = aTexCoord1;" \
			"	vFogFactor = clamp((uFogSettings.x - length(uCameraPos - aPos)) * uFogSettings.y, 0.0, 1.0);" \
			"	vLightColor = clamp(uAmbient + uDiffuse * max(dot(aNormal, uLightDir), 0.0), 0.0, 1.0);" \
			"}"
		);

		s_terrainFragment.setSource(
			"precision " FRAGMENT_PRECISION " float;" \

			"varying vec2 vTexCoord0;" \
			"varying vec2 vTexCoord1;" \
			"varying float vFogFactor;" \
			"varying vec3 vLightColor;" \

			"uniform vec2 uLightMapOffset;" \
			"uniform vec3 uFogColor;" \
			"uniform sampler2D sTerrain;" \
			"uniform sampler2D sLightMap;" \

			"void main(void) {" \
			"	vec4 lightMapColor = texture2D(sLightMap, vTexCoord1 + uLightMapOffset);" \
			"	vec3 color = texture2D(sTerrain, vTexCoord0).rgb * lightMapColor.rgb * vLightColor * 2.0;" \
			"	gl_FragColor = vec4(mix(uFogColor, color, vFogFactor), lightMapColor.a);" \
			"}"
		);

		const GLuint attribs[] = {
			VATTRIB_POS,
			VATTRIB_NORMAL,
			VATTRIB_TEXCOORD0,
			VATTRIB_TEXCOORD1
		};

		terrain.link(&s_terrainFragment, &s_terrainVertex, attribs);

		terrain.uWVP = terrain.location("uWVP");
		terrain.uLightMapOffset = terrain.location("uLightMapOffset");
		terrain.uCameraPos = terrain.location("uCameraPos");
		terrain.uFogSettings = terrain.location("uFogSettings");
		terrain.uFogColor = terrain.location("uFogColor");
		terrain.uAmbient = terrain.location("uAmbient");
		terrain.uDiffuse = terrain.location("uDiffuse");
		terrain.uLightDir = terrain.location("uLightDir");

		terrain.use();

		gl::uniform(terrain.location("sTerrain"), 0);
		gl::uniform(terrain.location("sLightMap"), 1);
	}

	void createWaterProgram()
	{
		s_waterVertex.setSource(
			"attribute vec3 aPos;" \
			"attribute vec2 aTexCoord0;" \
			"attribute vec4 aDiffuse;" \

			"varying vec2 vTexCoord0;" \
			"varying vec4 vDiffuse;" \
			"varying float vFogFactor;" \

			"uniform mat4 uWVP;" \
			"uniform vec3 uCameraPos;" \
			"uniform vec2 uFogSettings;" \

			"void main(void) {" \
			"	gl_Position = uWVP * vec4(aPos, 1.0);" \
			"	vTexCoord0 = aTexCoord0;" \
			"	vDiffuse = aDiffuse;" \
			"	vFogFactor = clamp((uFogSettings.x - length(uCameraPos - aPos)) * uFogSettings.y, 0.0, 1.0);" \
			"}"
		);

		s_waterFragment.setSource(
			"precision " FRAGMENT_PRECISION " float;" \

			"varying vec2 vTexCoord0;" \
			"varying float vFogFactor;" \
			"varying vec4 vDiffuse;" \

			"uniform vec2 uTextureOffset;" \
			"uniform vec3 uFogColor;" \
			"uniform sampler2D sTexture;" \

			"void main(void) {" \
			"	vec3 color = texture2D(sTexture, (vTexCoord0 - floor(vTexCoord0)) * 0.25 + uTextureOffset).rgb * vDiffuse.rgb;" \
			"	gl_FragColor = vec4(mix(uFogColor, color, vFogFactor), 0.52);" \
			"}"
		);

		const GLuint attribs[] = {
			VATTRIB_POS,
			VATTRIB_TEXCOORD0,
			VATTRIB_DIFFUSE
		};

		water.link(&s_waterFragment, &s_waterVertex, attribs);

		water.uWVP = water.location("uWVP");
		water.uTextureOffset = water.location("uTextureOffset");
		water.uCameraPos = water.location("uCameraPos");
		water.uFogSettings = water.location("uFogSettings");
		water.uFogColor = water.location("uFogColor");
	}

	void createCloudProgram()
	{
		s_cloudVertex.setSource(
			"attribute vec3 aPos;" \
			"attribute vec2 aTexCoord0;" \

			"varying vec2 vTexCoord0;" \
			"varying float vFogFactor;" \

			"uniform mat4 uWVP;" \
			"uniform vec3 uCameraPos;" \
			"uniform vec2 uFogSettings;" \
			"uniform float uHeightOffset;" \

			"void main(void) {" \
			"	vec3 pos = aPos;" \
			"	pos.y += uHeightOffset;" \
			"	gl_Position = uWVP * vec4(pos, 1.0);" \
			"	vTexCoord0 = aTexCoord0;" \
			"	vFogFactor = clamp((uFogSettings.x - length(uCameraPos - pos)) * uFogSettings.y, 0.0, 1.0);" \
			"}"
		);

		s_cloudFragment.setSource(
			"precision " FRAGMENT_PRECISION " float;" \

			"varying vec2 vTexCoord0;" \
			"varying float vFogFactor;" \

			"uniform float uTextureOffset;" \
			"uniform vec3 uFogColor;" \
			"uniform sampler2D sTexture;" \

			"void main(void) {" \
			"	vec3 color = texture2D(sTexture, vTexCoord0 + vec2(uTextureOffset, uTextureOffset)).rgb;" \
			"	gl_FragColor = vec4(mix(uFogColor, color, vFogFactor), 0.52);" \
			"}"
		);

		const GLuint attribs[] = {
			VATTRIB_POS,
			VATTRIB_TEXCOORD0
		};

		cloud.link(&s_cloudFragment, &s_cloudVertex, attribs);

		cloud.uWVP = cloud.location("uWVP");
		cloud.uTextureOffset = cloud.location("uTextureOffset");
		cloud.uCameraPos = cloud.location("uCameraPos");
		cloud.uFogSettings = cloud.location("uFogSettings");
		cloud.uFogColor = cloud.location("uFogColor");
		cloud.uHeightOffset = cloud.location("uHeightOffset");
	}

	void createSkyboxProgram()
	{
		s_skyboxVertex.setSource(
			"attribute vec3 aPos;" \
			"attribute vec2 aTexCoord0;" \
			"attribute float aDiffuse;" \

			"varying vec2 vTexCoord0;" \
			"varying float vDiffuse;" \

			"uniform mat4 uWVP;" \
			"uniform float uAlphaFactor;" \

			"void main(void) {" \
			"	gl_Position = uWVP * vec4(aPos, 1.0);" \
			"	vTexCoord0 = aTexCoord0;" \
			"	vDiffuse = aDiffuse * uAlphaFactor;" \
			"}"
		);

		s_skyboxFragment.setSource(
			"precision " FRAGMENT_PRECISION " float;" \

			"varying vec2 vTexCoord0;" \
			"varying float vDiffuse;" \

			"uniform float uTextureOffset;" \
			"uniform sampler2D sTexture;" \

			"void main(void) {" \
			"	gl_FragColor = texture2D(sTexture, vTexCoord0 + vec2(0.0, uTextureOffset)) * vec4(1, 1, 1, vDiffuse);" \
			"}"
		);

		const GLuint attribs[] = {
			VATTRIB_POS,
			VATTRIB_TEXCOORD0,
			VATTRIB_DIFFUSE
		};

		skybox.link(&s_skyboxFragment, &s_skyboxVertex, attribs);

		skybox.uWVP = skybox.location("uWVP");
		skybox.uTextureOffset = skybox.location("uTextureOffset");
		skybox.uAlphaFactor = skybox.location("uAlphaFactor");
	}

	void createObjectProgram()
	{
		s_objectVertex.setSource(
			"attribute vec3 aPos;" \
			"attribute vec3 aNormal;" \
			"attribute vec2 aTexCoord0;" \

			"varying vec2 vTexCoord0;" \
			"varying float vFogFactor;" \
			"varying vec3 vLightColor;" \

			"uniform mat4 uWVP;" \
			"uniform mat4 uWorld;" \
			"uniform vec3 uCameraPos;" \
			"uniform vec2 uFogSettings;" \
			"uniform vec3 uAmbient;" \
			"uniform vec3 uDiffuse;" \
			"uniform vec3 uLightDir;" \
			"uniform bvec3 uEffects;" \

			"void main(void) {" \
			"	gl_Position = uWVP * vec4(aPos, 1.0);" \
			"	vTexCoord0 = aTexCoord0;" \
			"	vFogFactor = uEffects.x ? clamp((uFogSettings.x - length(uCameraPos - (uWorld * vec4(aPos, 1.0)).xyz)) * uFogSettings.y, 0.0, 1.0) : 1.0;" \
			"	vLightColor = uEffects.y ? clamp(uAmbient + uDiffuse * max(dot(normalize((uWorld * vec4(aNormal, 1.0)).xyz), uLightDir), 0.0), 0.0, 1.0) : vec3(1.0);" \
			"}"
		);

		s_skinVertex.setSource(
			"attribute vec3 aPos;" \
			"attribute vec3 aNormal;" \
			"attribute vec2 aTexCoord0;" \
			"attribute vec2 aWeight;" \
			"attribute vec2 aBoneId;" \

			"varying vec2 vTexCoord0;" \
			"varying float vFogFactor;" \
			"varying vec3 vLightColor;" \

			"uniform mat4 uWVP;" \
			"uniform mat4 uWorld;" \
			"uniform vec3 uCameraPos;" \
			"uniform vec2 uFogSettings;" \
			"uniform vec3 uAmbient;" \
			"uniform vec3 uDiffuse;" \
			"uniform vec3 uLightDir;" \
			"uniform bvec3 uEffects;" \
			"uniform mat4 uBones[" M_TOSTRING(MAX_SHADER_BONES) "];" \

			"void main(void) {" \
			"	vec4 pos = uBones[int(aBoneId.x)] * vec4(aPos, 1.0) * aWeight.x" \
			"		+ uBones[int(aBoneId.y)] * vec4(aPos, 1.0) * aWeight.y;" \
			"	pos.w = 1.0;" \

			"	vec4 normal = uBones[int(aBoneId.x)] * vec4(aNormal, 1.0) * aWeight.x" \
			"		+ uBones[int(aBoneId.y)] * vec4(aNormal, 1.0) * aWeight.y;" \
			"	normal = vec4(normalize(normal.xyz), 1.0);" \

			"	gl_Position = uWVP * pos;" \
			"	vTexCoord0 = aTexCoord0;" \
			"	vFogFactor = uEffects.x ? clamp((uFogSettings.x - length(uCameraPos - (uWorld * pos).xyz)) * uFogSettings.y, 0.0, 1.0) : 1.0;" \
			"	vLightColor = uEffects.y ? clamp(uAmbient + uDiffuse * max(dot(normalize((uWorld * normal).xyz), uLightDir), 0.0), 0.0, 1.0) : vec3(1.0);" \
			"}"
		);

		s_objectFragment.setSource(
			"precision " FRAGMENT_PRECISION " float;" \

			"varying vec2 vTexCoord0;" \
			"varying float vFogFactor;" \
			"varying vec3 vLightColor;" \

			"uniform vec3 uFogColor;" \
			"uniform sampler2D sTexture;" \
			"uniform bvec3 uEffects;" \

			"void main(void) {" \
			"	vec4 color = texture2D(sTexture, vTexCoord0);" \
			"	if(color.a < 0.69) discard;" \
			"	gl_FragColor = vec4(mix(uFogColor, color.rgb * vLightColor, vFogFactor), color.a);" \
			"}"
		);

		const GLuint normalAttribs[] = {
			VATTRIB_POS,
			VATTRIB_NORMAL,
			VATTRIB_TEXCOORD0
		};

		object.link(&s_objectFragment, &s_objectVertex, normalAttribs);

		object.uWVP = object.location("uWVP");
		object.uCameraPos = object.location("uCameraPos");
		object.uFogSettings = object.location("uFogSettings");
		object.uFogColor = object.location("uFogColor");
		object.uWorld = object.location("uWorld");
		object.uAmbient = object.location("uAmbient");
		object.uDiffuse = object.location("uDiffuse");
		object.uLightDir = object.location("uLightDir");
		object.uEffects = object.location("uEffects");

		object.curEffects = ivec3(false, false, false);

		const GLuint skinAttribs[] = {
			VATTRIB_POS,
			VATTRIB_WEIGHT,
			VATTRIB_BONEID,
			VATTRIB_NORMAL,
			VATTRIB_TEXCOORD0
		};

		skin.link(&s_objectFragment, &s_skinVertex, skinAttribs);

		skin.uWVP = skin.location("uWVP");
		skin.uCameraPos = skin.location("uCameraPos");
		skin.uFogSettings = skin.location("uFogSettings");
		skin.uFogColor = skin.location("uFogColor");
		skin.uWorld = skin.location("uWorld");
		skin.uAmbient = skin.location("uAmbient");
		skin.uDiffuse = skin.location("uDiffuse");
		skin.uLightDir = skin.location("uLightDir");
		skin.uEffects = skin.location("uEffects");
		skin.uBones = skin.location("uBones");

		skin.curEffects = ivec3(false, false, false);
	}

	void createSfxProgram()
	{
		s_sfxVertex.setSource(
			"attribute vec3 aPos;" \
			"attribute vec2 aTexCoord0;" \

			"varying vec2 vTexCoord0;" \

			"uniform mat4 uWVP;" \

			"void main(void) {" \
			"	gl_Position = uWVP * vec4(aPos, 1.0);" \
			"	vTexCoord0 = aTexCoord0;" \
			"}"
		);

		s_sfxFragment.setSource(
			"precision " FRAGMENT_PRECISION " float;" \

			"varying vec2 vTexCoord0;" \

			"uniform float uAlphaFactor;" \
			"uniform sampler2D sTexture;" \

			"void main(void) {" \
			"	gl_FragColor = texture2D(sTexture, vTexCoord0) * vec4(1, 1, 1, uAlphaFactor);" \
			"}"
		);

		const GLuint attribs[] = {
			VATTRIB_POS,
			VATTRIB_TEXCOORD0
		};

		sfx.link(&s_sfxFragment, &s_sfxVertex, attribs);

		sfx.uWVP = sfx.location("uWVP");
		sfx.uAlphaFactor = sfx.location("uAlphaFactor");
		sfx.alphaFactor = 0.0f;
	}

	void createRainProgram()
	{
		s_rainVertex.setSource(
			"attribute vec3 aPos;" \
			"attribute float aDiffuse;" \

			"varying float vDiffuse;" \

			"uniform mat4 uWVP;" \

			"void main(void) {" \
			"	gl_Position = uWVP * vec4(aPos, 1.0);" \
			"	vDiffuse = aDiffuse;" \
			"}"
		);

		s_rainFragment.setSource(
			"precision " FRAGMENT_PRECISION " float;" \

			"varying float vDiffuse;" \

			"void main(void) {" \
			"	gl_FragColor = vec4(0.5, 0.5, 0.5, vDiffuse);" \
			"}"
		);

		const GLuint attribs[] = {
			VATTRIB_POS,
			VATTRIB_DIFFUSE
		};

		rain.link(&s_rainFragment, &s_rainVertex, attribs);

		rain.uWVP = rain.location("uWVP");
	}

	void createSnowProgram()
	{
		s_snowVertex.setSource(
			"attribute vec3 aPos;" \

			"uniform mat4 uWVP;" \
			"uniform float uSizeFactor;" \
			"uniform vec3 uCameraPos;" \

			"void main(void) {" \
			"	gl_Position = uWVP * vec4(aPos, 1.0);" \
			"	gl_PointSize = (8.0 - clamp(length(uCameraPos - aPos), 0.0, 25.0) / 6.0) * uSizeFactor;" \
			"}"
		);

		s_snowFragment.setSource(
			"precision " FRAGMENT_PRECISION " float;" \

			"void main(void) {" \
			"	float dist = clamp(length(gl_PointCoord - vec2(0.5, 0.5)), 0.4, 0.5); " \
			"	gl_FragColor = vec4(1, 1, 1, 5.0 - dist * 10.0);" \
			"}"
		);

		const GLuint attribs[] = {
			VATTRIB_POS
		};

		snow.link(&s_snowFragment, &s_snowVertex, attribs);

		snow.uWVP = snow.location("uWVP");
		snow.uSizeFactor = snow.location("uSizeFactor");
		snow.uCameraPos = snow.location("uCameraPos");
	}

	void createRender2DProgram()
	{
		s_render2dVertex.setSource(
			"attribute vec2 aPos;" \
			"attribute vec2 aTexCoord0;" \
			"attribute vec4 aDiffuse;" \

			"varying vec2 vTexCoord0;" \
			"varying vec4 vDiffuse;" \

			"uniform mat4 uWVP;" \

			"void main(void) {" \
			"	gl_Position = uWVP * vec4(aPos, 0.5, 1.0);" \
			"	vTexCoord0 = aTexCoord0;" \
			"	vDiffuse = aDiffuse;" \
			"}"
		);

		s_render2dFragment.setSource(
			"precision " FRAGMENT_PRECISION " float;" \

			"varying vec2 vTexCoord0;" \
			"varying vec4 vDiffuse;" \

			"uniform sampler2D sTexture; " \

			"void main(void) {" \
			"	gl_FragColor = texture2D(sTexture, vTexCoord0) * vDiffuse;"  \
			"}"
		);

		const GLuint attribs[] = {
			VATTRIB_POS,
			VATTRIB_TEXCOORD0,
			VATTRIB_DIFFUSE
		};

		render2d.link(&s_render2dFragment, &s_render2dVertex, attribs);

		render2d.uWVP = render2d.location("uWVP");
	}

	void createAll()
	{
		createTerrainProgram();
		createWaterProgram();
		createCloudProgram();
		createSkyboxProgram();
		createObjectProgram();
		createSfxProgram();
		createRainProgram();
		createSnowProgram();
		createRender2DProgram();
	}
}