#pragma once

#include "WndControl.hpp"

class WndContainer : public WndControl
{
public:
	WndContainer();

public:
	virtual Wnd::ControlType type() const { return Wnd::Container; }
	virtual void insert(WndControl* control, int pos = -1);
	virtual void remove(WndControl* control);
};

class WndBoxContainer : public WndContainer
{
public:
	WndBoxContainer(Wnd::Orientation orientation);

	void setOrientation(Wnd::Orientation orientation);
	Wnd::Orientation orientation() const { return m_orientation; }

public:
	virtual Wnd::ControlType type() const { return Wnd::BoxContainer; }

protected:
	virtual ivec2 onArrange();

private:
	Wnd::Orientation m_orientation;
};