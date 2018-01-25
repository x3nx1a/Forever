#pragma once

#define MAX_SHADER_BONES 28
#define MAX_STREAM_BUFFERS 4

namespace ShaderVars
{
	// set by the world
	extern mat4 view;
	extern mat4 proj;
	extern mat4 invView;
	extern vec3 cameraPos;
	extern vec3 cameraDir;
	extern int MPU;
	extern vec3 fogColor;
	extern vec3 lightDir;
	extern float sunAngle;
	extern mat4 viewProj;
	extern vec3 playerPos;
	extern vec4 frustum[6];

	// set by the window
	extern irect viewport;
	extern float pixelRatio;

	// set by the app
	extern gl::Texture2D blankTexture;
	extern gl::VertexArray skyboxVAO, sfxVAO, rainVAO, snowVAO, render2dVAO, sunVAO, customSfxVAO;
	extern gl::IndexBuffer terrainIBO, quadsIBO;
	extern gl::VertexBuffer skyboxVBO, sfxVBO, rainVBO, snowVBO, render2dVBO, sunVBO, customSfxVBO;

	void initAll();
	void releaseAll();
}