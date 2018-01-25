#pragma once

#include "Texture.hpp"
#include "Weather.hpp"
#include "Vertex.hpp"

class World;

class Skybox
{
public:
	explicit Skybox(World* world);
	~Skybox();

	void update(int frameCount);
	void updateView();

	void render();
	void renderWeather();

	void initWeather();

private:
	struct Fall
	{
		vec3 pos;
		vec3 velocity;
	};

private:
	World* const m_world;
	TexturePtr m_skyTextures[3];
	TexturePtr m_cloudTextures[3];
	float m_cloudPos;
	Weather m_weather;
	Fall* m_falls;
	int m_fallCount;
	RainVertex* m_rainVertices;
	SnowVertex* m_snowVertices;
};