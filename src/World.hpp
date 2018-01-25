#pragma once

#include "Landscape.hpp"
#include "Weather.hpp"

#define MAX_CULL_LANDS 16
#define MAX_CULL_OBJ 5000
#define MAX_CULL_SFX 500

class Skybox;

class World : public Resource
{
public:
	explicit World(const string& name);
	virtual ~World();

	void update(int frameCount);
	void render();

	void setCameraPos(const vec3& pos);
	void setCameraTarget(const vec3& target);
	void setUpdateView();

	const string& name() const;
	bool inDoor() const;
	Weather weather() const;

	ivec2 posToLand(const vec3& v) const;
	bool landInWorld(const ivec2& p) const;
	bool vecInWorld(float x, float z) const;
	bool vecInWorld(const vec3& p) const;
	float getLandHeight_fast(float x, float z) const;
	float getLandHeight(float x, float z) const;
	const WaterHeight* getWaterHeight(int x, int z) const;
	const WaterHeight* getWaterHeight(const vec3& p) const;
	Landscape* getLandscape(const vec3& p) const;
	void getLandTri(float x, float z, vec3* out) const;
	vec3 getLandNormal(float x, float z) const;
	vec3 getLandRot(float x, float z) const;
	const vec3& cameraPos() const;
	const vec3& cameraTarget() const;

	bool addObject(Object* obj);
	void deleteObject(Object* obj);
	bool insertObjLink(Object* obj);
	void addObjArray(Object* obj);
	bool removeObjLink(Object* obj);
	void removeObjArray(Object* obj);

protected:
	void onLoad(BinaryReader reader);

private:
	void updateView();
	void renderTerrain();
	void renderWater();
	void cullObjects();
	void setLight();

private:
	ivec2 m_size;
	int m_MPU;
	string m_name;
	vec3 m_cameraPos;
	vec3 m_cameraTarget;
	bool m_updateView;
	LandscapePtr* m_lands;
	int m_visibilityLand;
	float m_farPlane;
	float m_fogStart, m_fogEnd;
	TexturePtr m_cloudTexture;
	float m_waterFrame;
	Landscape* m_cullLands[MAX_CULL_LANDS];
	int m_cullLandCount;
	vec2 m_cloudsPos;
	Skybox* m_skybox;
	bool m_inDoor;
	vec3 m_ambient, m_diffuse, m_lightDir;
	Object* m_cullObj[MAX_CULL_OBJ];
	int m_cullObjCount;
	Object* m_cullSfx[MAX_CULL_SFX];
	int m_cullSfxCount;
	Weather m_weather;
	vector<Object*> m_deleteObjs;
};

typedef RefCountedPtr<World> WorldPtr;

#include "World.inl"