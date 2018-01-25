#pragma once

#include "BinaryReader.hpp"

namespace Music
{
	void loadProject(BinaryReader& reader);

	void play(int id, bool loop);

	void updateSec(int secCount);

	int playingId();

	int playingAndLoopingId();
}