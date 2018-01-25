#include "StdAfx.hpp"
#include "Platform.hpp"

#include <emscripten/val.h>

using namespace emscripten;

namespace Platform
{
	void removeSplash()
	{
		val::global("Platform").call<void>("removeSplash");
	}

	DisplayProperties getDisplayProperties()
	{
		val props = val::global("Platform").call<val>("getDisplayProperties");

		DisplayProperties ret;
		ret.size.x = props["width"].as<int>();
		ret.size.y = props["height"].as<int>();
		ret.pixelRatio = props["pixelRatio"].as<float>();
		return ret;
	}

	void showFatalError()
	{
		val::global("Platform").call<void>("showFatalError");
	}

	void setMusicVolume(float volume)
	{
		val::global("Platform").call<void>("setMusicVolume", volume);
	}

	void playMusic(const string& filename, float offset)
	{
		val::global("Platform").call<void>("playMusic", filename, offset);
	}

	void setCursor(const string& filename)
	{
		val::global("Platform").call<void>("setCursor", filename);
	}
}