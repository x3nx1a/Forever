#include "StdAfx.hpp"
#include "WndControl.hpp"
#include "WndTheme.hpp"
#include "Render2D.hpp"
#include "WndManager.hpp"

WndControl* WndControl::s_mouseHover = nullptr;
WndControl* WndControl::s_buttonDown[Wnd::MouseButtonCount];
ivec2 WndControl::s_mousePos;

WndControl::WndControl()
	: m_parent(nullptr),
	m_cursor(Wnd::CurBase),
	m_flags(Wnd::Enabled | Wnd::Visible),
	m_size(0, 0),
	m_minSize(-1, -1),
	m_maxSize(-1, -1),
	m_alignment(Wnd::Center),
	m_themeControl(nullptr)
{
}

WndControl::~WndControl()
{
	setParent(nullptr);

	while (!m_children.empty())
		delete m_children.back();

	if (m_themeControl)
		delete m_themeControl;
}

void WndControl::setParent(WndControl* parent)
{
	if (parent != m_parent)
	{
		if (m_parent)
		{
			auto it = find(m_parent->m_children.begin(), m_parent->m_children.end(), this);
			if (it != m_parent->m_children.end())
				m_parent->m_children.erase(it);
		}

		m_parent = parent;

		if (m_parent)
		{
			m_parent->m_children.push_back(this);
			init();
		}
		else
			setFlag(Wnd::Visible, false);
	}
}

void WndControl::init()
{
	setFlag(Wnd::Init, false);

	if (m_themeControl)
	{
		delete m_themeControl;
		m_themeControl = nullptr;
	}

	if (!is(Wnd::Manager) && !is(Wnd::Container))
		m_themeControl = WndTheme::instance->createControl(this);

	onInitialize();

	setFlag(Wnd::Init, true);

	arrange();
}

void WndControl::setCursor(Wnd::Cursor cursor)
{
	if (m_cursor != cursor)
		m_cursor = cursor;
}

void WndControl::setFlag(Wnd::ControlFlags flag, bool add)
{
	if ((add && hasFlag(flag)) || (!add && !hasFlag(flag)))
		return;

	if (add)
		m_flags |= flag;
	else
		m_flags &= ~flag;

	if (!isInit())
		return;

	switch (flag)
	{
	case Wnd::Visible:
	case Wnd::Enabled:
		if (!add)
		{
			const bool updateMouseHover = (s_mouseHover == this) || (s_buttonDown[Wnd::ButtonLeft] == this);
			if (s_mouseHover == this)
				s_mouseHover = nullptr;
			for (int i = 0; i < Wnd::MouseButtonCount; i++)
				if (s_buttonDown[i] == this)
					s_buttonDown[i] = nullptr;
			if (updateMouseHover)
				WndManager::instance->updateMouseHover();
		}
		else
			WndManager::instance->updateMouseHover();
		break;
	default:
		break;
	}
}

void WndControl::arrange()
{
	ivec2 size = onArrange();

	size.x += m_padding.left + m_padding.right;
	size.y += m_padding.top + m_padding.bottom;

	if (m_minSize.x != -1 && size.x < m_minSize.x)
		size.x = m_minSize.x;
	else if (m_maxSize.x != -1 && size.x > m_maxSize.x)
		size.x = m_maxSize.x;

	if (m_minSize.y != -1 && size.y < m_minSize.y)
		size.y = m_minSize.y;
	else if (m_maxSize.y != -1 && size.y > m_maxSize.y)
		size.y = m_maxSize.y;

	if (m_size != size)
	{
		m_size = size;

		if (isInit())
		{
			onResize();
			if (m_parent && m_parent->isInit() && m_parent->visible())
				m_parent->arrange();
		}
	}
}

void WndControl::setPadding(const irect& padding)
{
	if (m_padding != padding)
	{
		m_padding = padding;
		if (isInit())
			arrange();
	}
}

void WndControl::setMargin(const irect& margin)
{
	if (m_margin != margin)
	{
		m_margin = margin;
		if (m_parent && m_parent->isInit() && m_parent->visible())
			m_parent->arrange();
	}
}

void WndControl::setAlignment(Wnd::Alignment alignement)
{
	if (m_alignment != alignement)
	{
		m_alignment = alignement;
		if (m_parent && m_parent->isInit() && m_parent->visible())
			m_parent->arrange();
	}
}

void WndControl::setFixedSize(const ivec2& size)
{
	m_minSize = size;
	m_maxSize = size;
	arrange();
}

void WndControl::setMinimumSize(const ivec2& size)
{
	m_minSize = size;
	arrange();
}

void WndControl::setMaximumSize(const ivec2& size)
{
	m_maxSize = size;
	arrange();
}

void WndControl::render() const
{
	if (hasFrame())
	{
		Render2D::setViewRect(irect(mapToGlobal(ivec2()), size()));
		onRender();
	}

	for (std::size_t i = 0; i < m_children.size(); i++)
	{
		const WndControl* const ctrl = m_children[i];

		if (ctrl->visible())
			ctrl->render();
	}
}

ivec2 WndControl::mapToGlobal(const ivec2& p) const
{
	if (m_parent)
		return p + m_pos + m_parent->mapToGlobal(m_parent->padding().leftTop());
	else
		return p + m_pos;
}

ivec2 WndControl::mapToParent(const ivec2& p) const
{
	if (m_parent)
		return p + m_pos + m_parent->padding().leftTop();
	else
		return p + m_pos;
}

void WndControl::move(const ivec2& pos)
{
	if (m_pos != pos)
	{
		m_pos = pos;
		onMove();
	}
}

WndControl* WndControl::findControlAt(const ivec2& pos) const
{
	if (contentsRect().contains(pos))
	{
		for (int i = (int)m_children.size() - 1; i >= 0; i--)
		{
			const WndControl* const child = m_children[i];

			if (child->visible())
			{
				WndControl* const ctrl = child->findControlAt(child->mapFromParent(pos));
				if (ctrl)
					return ctrl;
			}
		}
	}

	if (rect().contains(pos) && hasFrame())
		return (WndControl*)this;
	else
		return nullptr;
}

void WndControl::onInitialize()
{
	if (m_themeControl)
		m_themeControl->init();
}

void WndControl::onResize()
{
}

void WndControl::onRender() const
{
	if (m_themeControl)
		m_themeControl->render();
}

ivec2 WndControl::onArrange()
{
	if (m_themeControl)
		return m_themeControl->sizeHint();
	else
		return ivec2();
}

void WndControl::onMove()
{
}

bool WndControl::onMouseDown(Wnd::MouseButton button)
{
	return true;
}

void WndControl::onMouseUp(Wnd::MouseButton button)
{
}

void WndControl::onMouseEnter()
{
}

void WndControl::onMouseLeave()
{
}

void WndControl::onMouseMove(const ivec2& move)
{
}