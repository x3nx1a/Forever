#pragma once

#include "WndControl.hpp"
#include "WndContainer.hpp"

class WndWindow : public WndControl
{
public:
	WndWindow(const string& title);

	void setTitle(const string& title);
	const string& title() const { return m_title; }

	void setCentralControl(WndControl* control);
	WndControl* centralControl() const { return m_centralControl; }

	void show();

public:
	virtual Wnd::ControlType type() const { return Wnd::Window; }

protected:
	virtual ivec2 onArrange();

private:
	string m_title;
	WndControl* m_centralControl;
};

class WndButton : public WndControl
{
public:
	WndButton(const string& text);

	void setText(const string& text);
	const string& text() const { return m_text; }

public:
	virtual Wnd::ControlType type() const { return Wnd::Button; }

private:
	string m_text;
};