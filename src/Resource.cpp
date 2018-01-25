#include "StdAfx.hpp"
#include "Resource.hpp"
#include "Network.hpp"

#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#include "miniz.hpp"

namespace
{
	uint32_t s_uniqueId = 1;
}

void onResourceLoad(void* userArg, void* rawBuffer, int rawSize)
{
	Resource* const res = (Resource*)userArg;

	int uncompressedDataSize = 0;
	memcpy(&uncompressedDataSize, rawBuffer, sizeof(int));

	mz_ulong len = (mz_ulong)uncompressedDataSize;
	char* uncompressData = new char[len];
	mz_uncompress((unsigned char*)uncompressData, &len, ((const unsigned char*)rawBuffer) + sizeof(int), (mz_ulong)rawSize);

	res->onLoad(BinaryReader(uncompressData, uncompressedDataSize));

	delete[] uncompressData;

	res->m_loadState = Resource::Loaded;
	res->release();
}

void onResourceLoadError(void* userArg)
{
	Resource* const res = (Resource*)userArg;
	emscripten_log(EM_LOG_ERROR, "Failed to load resource '%s'", res->filename().c_str());
	res->m_loadState = Resource::NotLoaded;
	res->release();
}

Resource::Resource(const string& filename)
	: m_filename(filename),
	m_loadState(NotLoaded),
	m_uniqueId(s_uniqueId)
{
	s_uniqueId++;
}

Resource::Resource()
	: Resource("")
{
}

void Resource::startLoad()
{
	if (m_loadState == Loading)
		return;

	m_loadState = Loading;

	const string url = Network::resourcesPath() + m_filename;

	addRef();

	emscripten_async_wget_data(
		url.c_str(),
		this,
		onResourceLoad,
		onResourceLoadError
	);
}

void Resource::setFilename(const string& newFilename)
{
	m_filename = newFilename;
}