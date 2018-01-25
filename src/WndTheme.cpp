#include "StdAfx.hpp"
#include "WndTheme.hpp"
#include "WndTheme_v19.hpp"

WndTheme* WndTheme::instance = nullptr;

WndTheme* WndTheme::create(const string& name)
{
	return new WndTheme_v19();
}

WndTheme::WndTheme(const string& name)
	: m_name(name)
{
}

WndTheme::~WndTheme()
{
}

WndThemeControl::WndThemeControl(WndControl* ctrl)
	: m_control(ctrl)
{
}

WndThemeControl::~WndThemeControl()
{
}