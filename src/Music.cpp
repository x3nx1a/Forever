#include "StdAfx.hpp"
#include "Music.hpp"
#include "Config.hpp"
#include "Platform.hpp"
#include "Network.hpp"

namespace Music
{
	namespace
	{
		struct MusicProp
		{
			int id;
			char filename[64];
			int duration;
		};

		const MusicProp* s_curPlaying = nullptr;

		const MusicProp* s_curLoop = nullptr;

		int s_curOffset;

		float s_curVolume = 1.0f;

		vector<MusicProp> s_musicProps;
	}

	void loadProject(BinaryReader& reader)
	{
		const int musicCount = reader.read<int>();
		s_musicProps.resize(musicCount);

		for (int i = 0; i < musicCount; i++)
		{
			MusicProp& prop = s_musicProps[i];

			reader >> prop.id
				>> prop.duration;

			const int len = reader.read<int>();
			reader.read(prop.filename, len);
			prop.filename[len] = '\0';
		}
	}

	void play(int id, bool loop)
	{
		const MusicProp* prop = nullptr;

		for (std::size_t i = 0; i < s_musicProps.size(); i++)
		{
			if (s_musicProps[i].id == id)
			{
				prop = &s_musicProps[i];
				break;
			}
		}

		if (!prop)
		{
			emscripten_log(EM_LOG_ERROR, "No music with id %d found", id);
			return;
		}

		s_curPlaying = prop;
		s_curOffset = 0;

		if (loop)
			s_curLoop = prop;

		if (s_curVolume >= 0.001f && s_curPlaying)
			Platform::playMusic(Network::resourcesPath() + "music/" + s_curPlaying->filename, (float)s_curOffset);
	}

	void updateSec(int secCount)
	{
		if (s_curVolume != Config::musicVolume)
		{
			Platform::setMusicVolume(Config::musicVolume);

			if (s_curVolume < 0.001f && Config::musicVolume >= 0.001f && s_curPlaying)
				Platform::playMusic(Network::resourcesPath() + "music/" + s_curPlaying->filename, (float)s_curOffset);

			s_curVolume = Config::musicVolume;
		}

		if (s_curPlaying)
		{
			s_curOffset += secCount;

			if (s_curOffset >= s_curPlaying->duration)
			{
				s_curPlaying = s_curLoop;
				s_curOffset = 0;

				if (s_curVolume >= 0.001f && s_curPlaying)
					Platform::playMusic(Network::resourcesPath() + "music/" + s_curPlaying->filename, (float)s_curOffset);
			}
		}
	}

	int playingId()
	{
		if (s_curPlaying)
			return s_curPlaying->id;
		else
			return 0;
	}

	int playingAndLoopingId()
	{
		if (s_curPlaying && s_curPlaying == s_curLoop)
			return s_curPlaying->id;
		else
			return 0;
	}
}