#include "StdAfx.hpp"
#include "WndControls.hpp"
#include "Project.hpp"
#include "WndManager.hpp"

WndWindow::WndWindow(const string& title)
	: m_centralControl(nullptr)
{
	setTitle(title);
	setVisible(false);
}

void WndWindow::show()
{
	setParent(WndManager::instance);
	setVisible(true);
}

void WndWindow::setTitle(const string& title)
{
	const string realTitle = Project::instance->text(title);

	if (m_title != realTitle)
		m_title = realTitle;
}

void WndWindow::setCentralControl(WndControl* control)
{
	control->setParent(this);
	m_centralControl = control;
}

ivec2 WndWindow::onArrange()
{
	ivec2 size;

	if (m_centralControl && m_centralControl->visible())
		size += m_centralControl->size();

	return size;
}

WndButton::WndButton(const string& text)
{
	setText(text);
}

void WndButton::setText(const string& text)
{
	const string realText = Project::instance->text(text);

	if (m_text != realText)
		m_text = realText;
}