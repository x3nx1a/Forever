#pragma once

struct DisplayProperties
{
	ivec2 size;
	float pixelRatio;
};

namespace Window
{
	void onProjectLoad(bool success);

	const DisplayProperties& display();
}