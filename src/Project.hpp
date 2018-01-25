#pragma once

#include "Resource.hpp"

class Project : public Resource
{
public:
	explicit Project();

	uint8_t version() const;

	string text(const string& textId) const;

protected:
	virtual void onLoad(BinaryReader reader);

private:
	uint8_t m_version;

public:
	static Project* instance;
};