#pragma once

#include "WndControl.hpp"
#include "Image.hpp"

class WndManager : public WndControl
{
public:
	WndManager();

	void showLoginTitle();

public:
	virtual Wnd::ControlType type() const { return Wnd::Manager; }

public:
	void rootRender();
	void rootInit();
	void rootMouseMove(const ivec2& pos);
	void rootMouseDown(Wnd::MouseButton button);
	void rootMouseUp(Wnd::MouseButton button);

protected:
	virtual void onRender() const;
	virtual bool onMouseDown(Wnd::MouseButton button, const ivec2& pos);

private:
	void updateMouseHover();
	void updateCursor(Wnd::Cursor cursor);

private:
	Wnd::Cursor m_currentCursor;

public:
	static WndManager* instance;

	friend class WndControl;
};