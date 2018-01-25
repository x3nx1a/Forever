#include "StdAfx.hpp"
#include "ShaderVars.hpp"
#include "Landscape.hpp"
#include "Vertex.hpp"

namespace ShaderVars
{
	irect viewport;
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
	int MPU;
	vec3 fogColor;
	vec3 cameraDir;
	vec3 lightDir;
	float sunAngle;
	mat4 viewProj;
	mat4 invView;
	vec3 playerPos;
	float pixelRatio;

	vec4 frustum[6];

	gl::Texture2D blankTexture;
	gl::VertexArray skyboxVAO, sfxVAO, rainVAO, snowVAO, render2dVAO, sunVAO, customSfxVAO;
	gl::IndexBuffer terrainIBO, quadsIBO;
	gl::VertexBuffer skyboxVBO, sfxVBO, rainVBO, snowVBO, render2dVBO, sunVBO, customSfxVBO;

	void createTerrainIBO()
	{
		const int indexCount = (128 * 3) * NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE;
		uint16_t* indices = new uint16_t[indexCount];

		const uint16_t baseIndices[] = {
			0, 10, 1, 1, 10, 2, 2, 10, 11, 2, 11, 12, 2, 12, 3, 3, 12, 4, 4, 12, 13, 4, 13, 14, 4, 14, 5, 5, 14, 6, 6, 14, 15, 6, 15, 16, 6, 16, 7, 7, 16, 8,
			10, 20, 11, 11, 20, 12, 12, 20, 21, 12, 21, 22, 12, 22, 13, 13, 22, 14, 14, 22, 23, 14, 23, 24, 14, 24, 15, 15, 24, 16,
			20, 30, 21, 21, 30, 22, 22, 30, 31, 22, 31, 32, 22, 32, 23, 23, 32, 24, 30, 40, 31, 31, 40, 32,
			8, 16, 17, 16, 24, 25, 16, 25, 26, 16, 26, 17, 24, 32, 33, 24, 33, 34, 24, 34, 25, 25, 34, 26, 26, 34, 35,
			32, 40, 41, 32, 41, 42, 32, 42, 33, 33, 42, 34, 34, 42, 43, 34, 43, 44, 34, 44, 35, 40, 50, 41, 41, 50, 42, 42, 50, 51, 42, 51, 52, 42, 52, 43, 43, 52, 44, 44, 52, 53,
			50, 60, 51, 51, 60, 52, 52, 60, 61, 52, 61, 62, 52, 62, 53, 60, 70, 61, 61, 70, 62, 62, 70, 71, 70, 80, 71,
			0, 9, 10, 9, 18, 10, 10, 18, 19, 10, 19, 20, 18, 27, 28, 18, 28, 19, 19, 28, 20, 20, 28, 29, 20, 29, 30,
			27, 36, 28, 28, 36, 37, 28, 37, 38, 28, 38, 29, 29, 38, 30, 30, 38, 39, 30, 39, 40, 36, 45, 46, 36, 46, 37, 37, 46, 38, 38, 46, 47, 38, 47, 48, 38, 48, 39, 39, 48, 40,
			45, 54, 46, 46, 54, 55, 46, 55, 56, 46, 56, 47, 47, 56, 48, 54, 63, 64, 54, 64, 55, 55, 64, 56, 63, 72, 64,
			48, 49, 40, 40, 49, 50, 48, 56, 57, 48, 57, 58, 48, 58, 49, 49, 58, 50, 50, 58, 59, 50, 59, 60,
			56, 64, 65, 56, 65, 66, 56, 66, 57, 57, 66, 58, 58, 66, 67, 58, 67, 68, 58, 68, 59, 59, 68, 60, 60, 68, 69, 60, 69, 70,
			64, 72, 73, 64, 73, 74, 64, 74, 65, 65, 74, 66, 66, 74, 75, 66, 75, 76, 66, 76, 67, 67, 76, 68, 68, 76, 77, 68, 77, 78, 68, 78, 69, 69, 78, 70, 70, 78, 79, 70, 79, 80,
		};

		int Y, X, i;
		for (Y = 0; Y < NUM_PATCHES_PER_SIDE; Y++)
		{
			for (X = 0; X < NUM_PATCHES_PER_SIDE; X++)
			{
				const uint16_t baseVertex = ((PATCH_SIZE + 1) * (PATCH_SIZE + 1)) * (Y * NUM_PATCHES_PER_SIDE + X);

				uint16_t* const patchIndices = indices + (128 * 3) * (Y * NUM_PATCHES_PER_SIDE + X);

				for (i = 0; i < (128 * 3); i++)
					patchIndices[i] = baseIndices[i] + baseVertex;
			}
		}

		gl::VertexArray tempVAO;
		tempVAO.create();
		tempVAO.bind();

		terrainIBO.create();
		terrainIBO.bind(tempVAO);
		terrainIBO.data(indexCount * sizeof(uint16_t), indices);
		delete[] indices;
	}

	void createBlankTexture()
	{
		const u8vec3 data(255, 255, 255);

		blankTexture.create();
		blankTexture.bind();
		blankTexture.image(0, GL_RGB, 1, 1, &data);
		blankTexture.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		blankTexture.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	void createQuadsIBO()
	{
		const int indexCount = 6 * 128;
		uint16_t* indexes = new uint16_t[indexCount];

		for (int i = 0; i < indexCount / 6; i++)
		{
			indexes[i * 6 + 0] = i * 4 + 0;
			indexes[i * 6 + 1] = i * 4 + 1;
			indexes[i * 6 + 2] = i * 4 + 2;

			indexes[i * 6 + 3] = i * 4 + 1;
			indexes[i * 6 + 4] = i * 4 + 2;
			indexes[i * 6 + 5] = i * 4 + 3;
		}

		gl::VertexArray tempVAO;
		tempVAO.create();
		tempVAO.bind();

		quadsIBO.create();
		quadsIBO.bind(tempVAO);
		quadsIBO.data(indexCount * sizeof(uint16_t), indexes);
		delete[] indexes;
	}

	void createSkyboxVAO()
	{
		const int vertexCount = 12 * 28 + 30;
		SkyboxVertex* vertices = new SkyboxVertex[vertexCount];

		SkyboxVertex* v = vertices;

		const float customFactor = 0.1f;
		const float radius = 110.0f * customFactor;

		for (int i = 0; i < 28; i++)
		{
			const float x1 = (float)sin(2 * pi<float>() / 28 * i) * radius;
			const float z1 = (float)cos(2 * pi<float>() / 28 * i) * radius;
			const float x2 = (float)sin(2 * pi<float>() / 28 * (i + 1)) * radius;
			const float z2 = (float)cos(2 * pi<float>() / 28 * (i + 1)) * radius;
			const float fu1 = 1.0f / 28.0f * i;
			const float fu2 = 1.0f / 28.0f * (i + 1);

			v->p = vec3(x1, 150.0f * customFactor, z1);
			v->tu1 = fu1;
			v->tv1 = 1.0f;
			v->a = 0.0f;
			v++;

			v->p = vec3(x1, 0.0f * customFactor, z1);
			v->tu1 = fu1;
			v->tv1 = 0.1f;
			v->a = 1.0f;
			v++;

			v->p = vec3(x2, 150.0f * customFactor, z2);
			v->tu1 = fu2;
			v->tv1 = 1.0f;
			v->a = 0.0f;
			v++;

			v->p = vec3(x2, 150.0f * customFactor, z2);
			v->tu1 = fu2;
			v->tv1 = 1.0f;
			v->a = 0.0f;
			v++;

			v->p = vec3(x1, 0.0f * customFactor, z1);
			v->tu1 = fu1;
			v->tv1 = 0.1f;
			v->a = 1.0f;
			v++;

			v->p = vec3(x2, 0.0f * customFactor, z2);
			v->tu1 = fu2;
			v->tv1 = 0.1f;
			v->a = 1.0f;
			v++;

			v->p = vec3(x1, 0.0f * customFactor, z1);
			v->tu1 = fu1;
			v->tv1 = 0.1f;
			v->a = 1.0f;
			v++;

			v->p = vec3(x1, -20.0f * customFactor, z1);
			v->tu1 = fu1;
			v->tv1 = 0.0f;
			v->a = 0.0f;
			v++;

			v->p = vec3(x2, 0.0f * customFactor, z2);
			v->tu1 = fu2;
			v->tv1 = 0.1f;
			v->a = 1.0f;
			v++;

			v->p = vec3(x2, 0.0f * customFactor, z2);
			v->tu1 = fu2;
			v->tv1 = 0.1f;
			v->a = 1.0f;
			v++;

			v->p = vec3(x1, -20.0f * customFactor, z1);
			v->tu1 = fu1;
			v->tv1 = 0.0f;
			v->a = 0.0f;
			v++;

			v->p = vec3(x2, -20.0f * customFactor, z2);
			v->tu1 = fu2;
			v->tv1 = 0.0f;
			v->a = 0.0f;
			v++;
		}

		v->p = vec3(0.0f, 100.0f * customFactor, 0.0f);
		v->tu1 = 1.0f;
		v->tv1 = 0.0f;
		v->a = 1.0f;
		v++;

		for (int i = 1; i < 30; i++)
		{
			const float x = (float)sin(2 * pi<float>() / 28 * i) * radius;
			const float z = (float)cos(2 * pi<float>() / 28 * i) * radius;

			v->p = vec3(x, 40.0f * customFactor, z);
			v->tu1 = (x + radius) / radius;
			v->tv1 = 1.0f - (z + radius) / radius;
			v->a = 0.0f;
			v++;
		}

		skyboxVBO.create();
		skyboxVBO.bind();
		skyboxVBO.data(vertexCount * sizeof(SkyboxVertex), vertices);
		delete[] vertices;

		skyboxVAO.create();
		skyboxVAO.bind();
		skyboxVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(SkyboxVertex), 0);
		skyboxVAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(SkyboxVertex), sizeof(vec3));
		skyboxVAO.vertexAttribPointer<float>(VATTRIB_DIFFUSE, false, sizeof(SkyboxVertex), sizeof(vec3) + sizeof(vec2));

		sunVBO.create();
		sunVBO.bind();

		sunVAO.create();
		sunVAO.bind();
		sunVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(SkyboxVertex), 0);
		sunVAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(SkyboxVertex), sizeof(vec3));
		sunVAO.vertexAttribPointer<float>(VATTRIB_DIFFUSE, false, sizeof(SkyboxVertex), sizeof(vec3) + sizeof(vec2));

		rainVBO.create();
		rainVBO.bind();

		rainVAO.create();
		rainVAO.bind();
		rainVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(RainVertex), 0);
		rainVAO.vertexAttribPointer<float>(VATTRIB_DIFFUSE, false, sizeof(RainVertex), sizeof(vec3));

		snowVBO.create();
		snowVBO.bind();

		snowVAO.create();
		snowVAO.bind();
		snowVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(SnowVertex), 0);
	}

	void createSfxVAO()
	{
		const int vertexCount = 4;
		SfxVertex* vertices = new SfxVertex[vertexCount];

		SfxVertex* v = vertices;

		v->p = vec3(-0.5f, 0.5f, 0.0f);
		v->t = vec2(0.0f, 1.0f);
		v++;
		v->p = vec3(0.5f, 0.5f, 0.0f);
		v->t = vec2(1.0f, 1.0f);
		v++;
		v->p = vec3(0.5f, -0.5f, 0.0f);
		v->t = vec2(1.0f, 0.0f);
		v++;
		v->p = vec3(-0.5f, -0.5f, 0.0f);
		v->t = vec2(0.0f, 0.0f);

		sfxVBO.create();
		sfxVBO.data(vertexCount * sizeof(SfxVertex), vertices);
		delete[] vertices;

		sfxVAO.create();
		sfxVAO.bind();
		sfxVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(SfxVertex), 0);
		sfxVAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(SfxVertex), sizeof(vec3));

		customSfxVBO.create();
		customSfxVBO.bind();

		customSfxVAO.create();
		customSfxVAO.bind();
		customSfxVAO.vertexAttribPointer<vec3>(VATTRIB_POS, false, sizeof(SfxVertex), 0);
		customSfxVAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(SfxVertex), sizeof(vec3));
	}

	void createRender2dVAO()
	{
		render2dVBO.create();
		render2dVBO.bind();

		render2dVAO.create();
		render2dVAO.bind();
		quadsIBO.bind(render2dVAO);
		render2dVAO.vertexAttribPointer<vec2>(VATTRIB_POS, false, sizeof(Vertex2D), 0);
		render2dVAO.vertexAttribPointer<vec2>(VATTRIB_TEXCOORD0, false, sizeof(Vertex2D), sizeof(vec2));
		render2dVAO.vertexAttribPointer<u8vec4>(VATTRIB_DIFFUSE, true, sizeof(Vertex2D), sizeof(vec2) * 2);
	}

	void initAll()
	{
		createTerrainIBO();
		createBlankTexture();
		createQuadsIBO();
		createSkyboxVAO();
		createSfxVAO();
		createRender2dVAO();
	}

	void releaseAll()
	{
		blankTexture.destroy();
		skyboxVAO.destroy(); sfxVAO.destroy(); rainVAO.destroy(); snowVAO.destroy(); render2dVAO.destroy(); sunVAO.destroy(); customSfxVAO.destroy();
		terrainIBO.destroy(); quadsIBO.destroy();
		skyboxVBO.destroy(); sfxVBO.destroy(); rainVBO.destroy(); snowVBO.destroy(); render2dVBO.destroy(); sunVBO.destroy(); customSfxVBO.destroy();
	}
}