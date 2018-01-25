#pragma once

#define M_TOSTRING_HELPER(v) #v
#define M_TOSTRING(v) M_TOSTRING_HELPER(v)

#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>
#include <list>

using namespace std;

#include <emscripten/emscripten.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SSE2
#define GLM_FORCE_LEFT_HANDED

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"

using namespace glm;

#include "Rectangle.hpp"