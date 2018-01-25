#pragma once

inline vec3 transformCoord(const vec3& v, const mat4& m)
{
	const vec4 temp = m * vec4(v, 1);
	return vec3(temp.x, temp.y, temp.z) / temp.w;
}

inline vec4 planeFromPoints(const vec3& v1, const vec3& v2, const vec3& v3)
{
	const vec3 normal = normalize(cross(v2 - v1, v3 - v1));
	return vec4(normal, -dot(v1, normal));
}