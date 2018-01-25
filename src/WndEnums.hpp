#pragma once

class WndControl;
class WndContainer;

namespace Wnd
{
	enum
	{
		None = 0
	};

	enum Cursor
	{
		CurBase = 1,
		CurDelay
	};

	enum Orientation
	{
		Vertical = 1,
		Horizontal
	};

	enum Alignment
	{
		Left = 1 << 0,
		Right = 1 << 1,
		Top = 1 << 2,
		Bottom = 1 << 3,
		VCenter = 1 << 4,
		HCenter = 1 << 5,

		TopLeft = Top | Left,
		TopRight = Top | Right,
		BottomLeft = Bottom | Left,
		BottomRight = Bottom | Right,
		Center = HCenter | VCenter,

		Default = Center
	};

	enum ControlFlags
	{
		Visible = 1 << 0,
		Enabled = 1 << 1,
		NoFrame = 1 << 2,
		Init = 1 << 3,
		MouseTracking = 1 << 4
	};

	enum ControlType
	{
		Container = (1 << 0),
		Window = (1 << 1),
		Button = (1 << 2),
		BoxContainer = Container | (1 << 3),
		Manager = (1 << 4)
	};

	enum MouseButton
	{
		ButtonLeft,
		ButtonRight,
		ButtonMiddle,
		MouseButtonCount
	};
}