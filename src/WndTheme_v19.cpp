#include "StdAfx.hpp"
#include "WndTheme_v19.hpp"
#include "WndControls.hpp"
#include "Render2D.hpp"

class WndThemeControlButton_v19 : public WndThemeControl
{
public:
	WndThemeControlButton_v19(WndControl* ctrl)
		: WndThemeControl(ctrl)
	{
	}

	virtual void init()
	{
		control()->setMargin(irect(6, 6, 6, 6));
	}

	virtual void render()
	{
		Render2D::fillRect(control()->rect(), u8vec4(255));
	}

	virtual ivec2 sizeHint()
	{
		return Render2D::measureString(control<WndButton>()->text(), 12);
	}
};

WndTheme_v19::WndTheme_v19()
	: WndTheme("v19")
{
}

WndThemeControl* WndTheme_v19::createControl(WndControl* control)
{
	if (control->is(Wnd::Button))
		return new WndThemeControlButton_v19(control);
	return nullptr;
}