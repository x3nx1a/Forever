#pragma once

class WndControl;

class WndThemeControl
{
public:
	WndThemeControl(WndControl* ctrl);
	virtual ~WndThemeControl();

	template<typename T = WndControl>
	T* control() const { return (T*)m_control; }

public:
	virtual void init() = 0;
	virtual void render() = 0;
	virtual ivec2 sizeHint() = 0;

private:
	WndControl* const m_control;
};

class WndTheme
{
public:
	virtual ~WndTheme();

	const string& name() const { return m_name; }

public:
	virtual WndThemeControl* createControl( WndControl* control) = 0;

protected:
	WndTheme(const string& name);

private:
	const string m_name;

public:
	WndTheme(const WndTheme&) = delete;
	WndTheme& operator=(const WndTheme&) = delete;

public:
	static WndTheme* instance;

public:
	static WndTheme* create(const string& name);
};