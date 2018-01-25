#pragma once

#include "WndEnums.hpp"

class WndThemeControl;

class WndControl
{
public:
	WndControl();
	virtual ~WndControl();

public:
	Wnd::Cursor cursor() const { return m_cursor; }
	bool enabled() const { return hasFlag(Wnd::ControlFlags::Enabled); }
	bool visible() const { return hasFlag(Wnd::ControlFlags::Visible); }
	WndControl* parent() const { return m_parent; }
	const ivec2& size() const { return m_size; }
	const ivec2& minimumSize() const { return m_minSize; }
	const ivec2& maximumSize() const { return m_maxSize; }
	const irect& margin() const { return m_margin; }
	const irect& padding() const { return m_padding; }
	bool is(Wnd::ControlType t) const { return (type() & t) != 0; }
	const ivec2& pos() const { return m_pos; }
	ivec2 mapToGlobal(const ivec2& p) const;
	ivec2 mapToParent(const ivec2& p) const;
	ivec2 mapFromGlobal(const ivec2& p) const { return p - mapToGlobal(ivec2()); }
	ivec2 mapFromParent(const ivec2& p) const { return p - mapToParent(ivec2()); }
	bool hasFrame() const { return !hasFlag(Wnd::NoFrame); }
	bool isInit() const { return hasFlag(Wnd::Init); }
	irect rect() const { return irect(0, 0, m_size.x, m_size.y); }
	irect contentsRect() const { return irect(m_padding.left, m_padding.top, m_size.x - m_padding.right, m_size.y - m_padding.bottom); }
	int width() const { return m_size.x; }
	int height() const { return m_size.y; }
	const vector<WndControl*>& children() const { return m_children; }
	Wnd::Alignment alignment() const { return m_alignment; }
	bool mouseHover() const { return s_mouseHover == this; }
	bool mouseButtonDown(Wnd::MouseButton button) const { return s_buttonDown[button] == this; }
	bool mouseTracking() const { return hasFlag(Wnd::MouseTracking); }
	WndThemeControl* themeControl() const { return m_themeControl; }

public:
	void setCursor(Wnd::Cursor cursor);
	void setFixedSize(const ivec2& size);
	void setMinimumSize(const ivec2& size);
	void setMaximumSize(const ivec2& size);
	void setVisible(bool visible) { setFlag(Wnd::Visible, visible); }
	void setParent(WndControl* parent);
	void setNoFrame(bool noFrame) { setFlag(Wnd::NoFrame, noFrame); }
	void setPadding(const irect& padding);
	void setMargin(const irect& margin);
	void setAlignment(Wnd::Alignment alignement);
	void move(const ivec2& pos);
	void setMouseTracking(bool mouseTracking) { setFlag(Wnd::MouseTracking, mouseTracking); }

public:
	virtual Wnd::ControlType type() const = 0;

protected:
	virtual void onInitialize();
	virtual void onResize();
	virtual void onRender() const;
	virtual ivec2 onArrange();
	virtual void onMove();
	virtual bool onMouseDown(Wnd::MouseButton button);
	virtual void onMouseUp(Wnd::MouseButton button);
	virtual void onMouseEnter();
	virtual void onMouseLeave();
	virtual void onMouseMove(const ivec2& move);

private:
	bool hasFlag(Wnd::ControlFlags flag) const { return (flag & m_flags) != 0; }

private:
	void init();
	void setFlag(Wnd::ControlFlags flag, bool add);
	void arrange();
	void render() const;
	WndControl* findControlAt(const ivec2& pos) const;

private:
	WndControl* m_parent;
	Wnd::Cursor m_cursor;
	uint32_t m_flags;
	vector<WndControl*> m_children;
	ivec2 m_size, m_minSize, m_maxSize;
	irect m_margin, m_padding;
	ivec2 m_pos;
	Wnd::Alignment m_alignment;
	WndThemeControl* m_themeControl;

private:
	WndControl(const WndControl&) = delete;
	WndControl& operator=(const WndControl&) = delete;

private:
	static WndControl* s_mouseHover;
	static WndControl* s_buttonDown[Wnd::MouseButtonCount];
	static ivec2 s_mousePos;

	friend class WndManager;
};