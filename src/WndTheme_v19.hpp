#pragma once

#include "WndTheme.hpp"

class WndTheme_v19 : public WndTheme
{
public:
	WndTheme_v19();

public:
	virtual WndThemeControl* createControl( WndControl* control);
};