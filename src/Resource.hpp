#pragma once

#include "RefCounted.hpp"
#include "BinaryReader.hpp"

class Resource : public RefCounted
{
public:
	enum LoadState
	{
		NotLoaded,
		Loading,
		Loaded
	};

public:
	Resource(const string& filename);
	Resource();

public:
	void startLoad();

	const string& filename() {
		return m_filename;
	}
	bool loaded() const {
		return m_loadState == Loaded;
	}
	LoadState loadState() const {
		return m_loadState;
	}
	uint32_t uniqueId() const {
		return m_uniqueId;
	}

	void setFilename(const string& newFilename);

protected:
	virtual void onLoad(BinaryReader reader) = 0;

private:
	const uint32_t m_uniqueId;
	 string m_filename;
	LoadState m_loadState;

private:
	friend void onResourceLoad(void*, void*, int);
	friend void onResourceLoadError(void*);
};