#include "StdAfx.hpp"
#include "WndManager.hpp"
#include "Music.hpp"
#include "WndLogin.hpp"
#include "Render2D.hpp"
#include "TextureManager.hpp"
#include "Platform.hpp"

#include "../gameres/defineSound.h"

WndManager* WndManager::instance = nullptr;

WndManager::WndManager()
	: m_currentCursor(Wnd::CurDelay)
{
	memset(s_buttonDown, 0, sizeof(s_buttonDown));
}

void WndManager::showLoginTitle()
{
	WndLogin* wndLogin = new WndLogin();
	wndLogin->show();

	if (Music::playingAndLoopingId() != BGM_TITLE)
		Music::play(BGM_TITLE, true);
}

void WndManager::rootRender()
{
	render();

	Render2D::flush();
}

void WndManager::rootInit()
{
	init();
	updateMouseHover();
}

void WndManager::rootMouseMove(const ivec2& pos)
{
	if (s_mousePos != pos)
	{
		const ivec2 diff = s_mousePos - pos;
		s_mousePos = pos;

		updateMouseHover();

		if (s_mouseHover && (s_mouseHover == s_buttonDown[Wnd::ButtonLeft] || s_mouseHover->mouseTracking()))
			s_mouseHover->onMouseMove(diff);
	}
}

void WndManager::rootMouseDown(Wnd::MouseButton button)
{
	if (s_buttonDown[button])
		return;

	WndControl* const ctrl = findControlAt(s_mousePos);
	if (ctrl->enabled())
	{
		s_buttonDown[button] = ctrl;
		if (!ctrl->onMouseDown(button))
			s_buttonDown[button] = nullptr;
	}
}

void WndManager::rootMouseUp(Wnd::MouseButton button)
{
	WndControl* const ctrl = s_buttonDown[button];

	if (ctrl)
	{
		ctrl->onMouseUp(button);
		s_buttonDown[button] = nullptr;

		if (button == Wnd::ButtonLeft)
			updateMouseHover();
	}
}

void WndManager::updateMouseHover()
{
	if (!s_buttonDown[Wnd::ButtonLeft])
	{
		WndControl* mouseHover = findControlAt(s_mousePos);
		if (!mouseHover->enabled())
			mouseHover = nullptr;

		if (s_mouseHover != mouseHover)
		{
			WndControl* oldMouseHover = s_mouseHover;
			s_mouseHover = mouseHover;

			if (oldMouseHover)
				oldMouseHover->onMouseLeave();

			if (s_mouseHover)
			{
				s_mouseHover->onMouseEnter();
				updateCursor(s_mouseHover->cursor());
			}
			else
				updateCursor(Wnd::CurBase);
		}
	}
}

void WndManager::updateCursor(Wnd::Cursor cursor)
{
	if (m_currentCursor == cursor)
		return;

	string filename;

	switch ((int)cursor)
	{
	case Wnd::None:
		break;
	case Wnd::CurDelay:
		filename = "curdelay";
		break;
	default:
		filename = "curbase";
		break;
	}

	Platform::setCursor(filename);
	m_currentCursor = cursor;
}

void WndManager::onRender() const
{
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	gl::enableDepthWrite();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

	const Image background = ImageManager::image("screen");
	const Image logo = ImageManager::image("logo");
	const Image char1 = ImageManager::image("screen_char1");

	float scale, scale2;
	ivec2 pos;

	if ((float)width() / (float)height() < (float)background.width() / (float)background.height())
	{
		scale = (float)height() / (float)background.height();
		scale2 = (float)width() / (float)background.width();
		pos.x = (int)((float)width() / 2.0f - scale * (float)background.width() / 2.0f);
	}
	else
	{
		scale = (float)width() / (float)background.width();
		scale2 = (float)height() / (float)background.height();
		pos.y = (int)((float)height() / 2.0f - scale * (float)background.height() / 2.0f);
	}

	Render2D::drawImage(background, pos, scale);

	static vec3 trans;
	trans += vec3(0.01f, 0.0f, 0.0f);
	if (trans.x > 1.0f) trans.x = 0.0f;
	if (trans.y > 1.0f) trans.y = 0.0f;
	if (trans.y > 1.0f) trans.y = 0.0f;

	const vec3 posFactor(1.0f - abs(trans.x * 2.0f - 1.0f), 0, 0);

	Render2D::drawImage(logo, vec2(
		(float)width() / 2.0f - scale2 * (float)logo.width() * 0.3f / 2.0f,
		(float)height() * 0.08f
	), scale2 * 0.3f);

	Render2D::drawImage(char1, vec2(
		(-3.5f + posFactor.x) * (float)char1.width() * 0.01f,
		(0.1f + posFactor.x) * (float)char1.height() * 0.01f
	) * scale, scale2 * 0.5f);
}

bool WndManager::onMouseDown(Wnd::MouseButton button, const ivec2& pos)
{
	return false;
}