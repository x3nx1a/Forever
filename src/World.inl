inline const string& World::name() const
{
	return m_name;
}

inline bool World::inDoor() const
{
	return m_inDoor;
}

inline ivec2 World::posToLand(const vec3& v) const
{
	return ivec2((int)v.x, (int)v.z) / (MAP_SIZE * m_MPU);
}

inline bool World::landInWorld(const ivec2& p) const
{
	return p.x >= 0 && p.y >= 0 && p.x < m_size.x && p.y < m_size.y;
}

inline Weather World::weather() const
{
	return m_weather;
}

inline bool World::vecInWorld(float x, float z) const
{
	return x >= 0.0f && ((int)x) < m_MPU * MAP_SIZE * m_size.x
		&& z >= 0.0f && ((int)z) < m_MPU * MAP_SIZE * m_size.y;
}

inline const WaterHeight* World::getWaterHeight(const vec3& p) const
{
	return getWaterHeight((int)p.x, (int)p.z);
}

inline bool World::vecInWorld(const vec3& p) const
{
	return vecInWorld(p.x, p.z);
}

inline const vec3& World::cameraPos() const
{
	return m_cameraPos;
}

inline const vec3& World::cameraTarget() const
{
	return m_cameraTarget;
}