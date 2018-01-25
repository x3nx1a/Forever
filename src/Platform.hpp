#pragma once

#include "Window.hpp"

namespace Platform
{
	void removeSplash();

	DisplayProperties getDisplayProperties();

	void showFatalError();

	void setMusicVolume(float volume);

	void playMusic(const string& filename, float offset);

	void setCursor(const string& filename);
}