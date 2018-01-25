#pragma once

#include "FileNode.hpp"

#define FILE_START_PACKAGE "data/client"

namespace File
{
	typedef void(*FreeCallback)(void*);

	struct LoadData
	{
		string baseName;
		string dir;
		string ext;
		FileNode node;
		FreeCallback freeCallback;
	};

	void initialize();
	void load(const char* filepath);

	void markPackagesUnused();
	void freeUnusedPackages();
}

namespace FileLoader
{
	void handle(File::LoadData& file);
}