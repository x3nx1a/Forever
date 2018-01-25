#include "StdAfx.hpp"
#include "WndContainer.hpp"

WndContainer::WndContainer()
{
	setNoFrame(true);
}

void WndContainer::insert(WndControl* control, int pos)
{
	control->setParent(this);
}

void WndContainer::remove(WndControl* control)
{
	control->setParent(nullptr);
}

WndBoxContainer::WndBoxContainer(Wnd::Orientation orientation)
{
	setOrientation(orientation);
}

void WndBoxContainer::setOrientation(Wnd::Orientation orientation)
{
	m_orientation = orientation;
}

ivec2 WndBoxContainer::onArrange()
{
	ivec2 size;

	const vector<WndControl*>& list = children();

	if (!list.empty())
	{
		int* positions = new int[list.size()];

		for (std::size_t i = 0; i < list.size(); i++)
		{
			const WndControl* const ctrl = list[i];

			size.x = glm::max(size.x, ctrl->width() + ctrl->margin().left + ctrl->margin().right);
			size.y += ctrl->margin().top;
			positions[i] = size.y;
			size.y += ctrl->height() + ctrl->margin().bottom;
		}

		for (std::size_t i = 0; i < list.size(); i++)
		{
			WndControl* const ctrl = list[i];

			ctrl->move(ivec2(ctrl->margin().left, positions[i]));
		}

		delete[] positions;
	}

	return size;
}