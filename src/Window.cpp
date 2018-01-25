#include "StdAfx.hpp"
#include "Window.hpp"
#include "Shaders.hpp"
#include "GameTime.hpp"
#include "Project.hpp"
#include "Platform.hpp"
#include "Canvas2D.hpp"
#include "Config.hpp"
#include "Music.hpp"
#include "World.hpp"

#include <emscripten/html5.h>
#include <ctime>

namespace Window
{
	namespace
	{
		DisplayProperties s_display;
		double s_lastUpdate = 0.0;
		double s_frameCount = 0.0;
		time_t s_lastUpdateSec;
		bool s_active = true;
		bool s_running = false;

		World* s_world = nullptr;
		bool s_rotate = false;
		ivec2 s_lastMousePos;
		vec2 s_cameraAngle;
		vec3 s_orientation;
	}

	void onFrame()
	{
		if (!s_running)
			return;

		const double now = emscripten_get_now();
		s_frameCount += (now - s_lastUpdate) / (1000.0 / 60.0);
		s_lastUpdate = now;

		if (s_frameCount >= 1.0)
		{
			const int frameCount = (int)s_frameCount;
			s_frameCount -= frameCount;

			GameTime::update();
			s_world->update(frameCount);
		}

		const time_t nowSec = time(nullptr);
		if (nowSec > s_lastUpdateSec)
		{
			const int secCount = (int)(nowSec - s_lastUpdateSec);

			Music::updateSec(secCount);

			s_lastUpdateSec = nowSec;
		}

		if (s_active && gl::isContextActive())
		{
			s_world->render();

			Image img = ImageManager::image("logo");
			Canvas2D::drawImage(img, s_display.size / 2);

			Canvas2D::flush();
		}
	}

	EM_BOOL onResize(int eventType, const EmscriptenUiEvent* uiEvent, void* userData)
	{
		s_display = Platform::getDisplayProperties();
		s_display.pixelRatio = 1.25f / s_display.pixelRatio;
		if (s_display.pixelRatio < 1.0f)
			s_display.pixelRatio = 1.0f;

		ShaderVars::viewport = irect(0, 0, s_display.size.x, s_display.size.y);
		ShaderVars::pixelRatio = s_display.pixelRatio;

		emscripten_set_canvas_size(s_display.size.x, s_display.size.y);
		glViewport(0, 0, s_display.size.x, s_display.size.y);
		Canvas2D::updateViewport();

		if (s_world)
			s_world->setUpdateView();

		return true;
	}

	void updateCameraAngle()
	{
		if (!s_world)
			return;

		const float phi = radians(s_cameraAngle.x);
		const float theta = radians(s_cameraAngle.y);
		s_orientation.x = cos(phi) * sin(theta);
		s_orientation.y = sin(phi);
		s_orientation.z = cos(phi) * cos(theta);

		s_world->setCameraTarget(s_world->cameraPos() + s_orientation);
	}

	EM_BOOL onVisibilityChange(int eventType, const EmscriptenVisibilityChangeEvent* visibilityChangeEvent, void* userData)
	{
		s_active = (visibilityChangeEvent->hidden == 0);
		return true;
	}

	EM_BOOL onMouseMove(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
	{
		if (gl::isContextActive() && s_rotate)
		{
			const ivec2 mousePos(mouseEvent->canvasX, mouseEvent->canvasY);
			const ivec2 mouseMove = mousePos - s_lastMousePos;
			s_lastMousePos = mousePos;

			s_cameraAngle += vec2(-mouseMove.y, mouseMove.x) / 2.5f;
			updateCameraAngle();
		}
		return true;
	}

	EM_BOOL onMouseDown(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
	{
		if (gl::isContextActive())
		{
			s_lastMousePos = ivec2(mouseEvent->canvasX, mouseEvent->canvasY);
			if (mouseEvent->button == 2)
				s_rotate = true;
		}
		return true;
	}

	EM_BOOL onMouseUp(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
	{
		if (mouseEvent->button == 2)
			s_rotate = false;
		return true;
	}

	EM_BOOL onMouseWheel(int eventType, const EmscriptenWheelEvent* wheelEvent, void* userData)
	{
		if (!gl::isContextActive())
			return true;

		if (s_world)
		{
			float factor = -(float)wheelEvent->deltaY;
			if (wheelEvent->deltaMode == DOM_DELTA_PIXEL)
				factor /= 50.0f;

			s_world->setCameraPos(s_world->cameraPos() + s_orientation * factor);
			updateCameraAngle();
		}

		return true;
	}

	EM_BOOL onContextLost(int eventType, const void* reserved, void* userData)
	{
		gl::loseContext();
		ShaderVars::releaseAll();

		return true;
	}

	EM_BOOL onContextRestored(int eventType, const void* reserved, void* userData)
	{
		if (!gl::restoreContext())
		{
			emscripten_cancel_main_loop();
			Platform::showFatalError();
		}

		return true;
	}

	const char* onCleanup(int eventType, const void* reserved, void* userData)
	{
		gl::loseContext();

		if (s_world)
		{
			delete s_world;
			s_world = nullptr;
		}
		if (Project::instance)
		{
			delete Project::instance;
			Project::instance = nullptr;
		}

		s_running = false;
		s_active = false;

		gl::destroyContext();

		return nullptr;
	}

	void onProjectLoad(bool success)
	{
		Platform::removeSplash();

		if (!success || !gl::createContext("#canvas") || !gl::restoreContext())
		{
			emscripten_cancel_main_loop();
			Platform::showFatalError();
			return;
		}

		emscripten_set_resize_callback("#window", 0, false, Window::onResize);
		emscripten_set_webglcontextlost_callback("#canvas", 0, false, onContextLost);
		emscripten_set_webglcontextrestored_callback("#canvas", 0, false, onContextRestored);
		emscripten_set_mousemove_callback("#canvas", 0, false, onMouseMove);
		emscripten_set_mousedown_callback("#canvas", 0, false, onMouseDown);
		emscripten_set_mouseup_callback("#canvas", 0, false, onMouseUp);
		emscripten_set_wheel_callback("#canvas", 0, false, onMouseWheel);

		Shaders::createAll();

		Music::updateSec(1);

		s_world = new World("wdmadrigal");
		s_world->setCameraPos(vec3(1200.0f, 120.0f, 1200.0f));
		updateCameraAngle();

		onResize(EMSCRIPTEN_EVENT_RESIZE, 0, 0);

		s_lastUpdate = emscripten_get_now();
		s_lastUpdateSec = time(nullptr);
		s_running = true;

		Platform::setCursor("curbase");
	}

	const DisplayProperties& display()
	{
		return s_display;
	}
}

int main()
{
	Project::instance = new Project();

	emscripten_set_beforeunload_callback(0, Window::onCleanup);
	emscripten_set_visibilitychange_callback(0, false, Window::onVisibilityChange);
	emscripten_set_main_loop(Window::onFrame, Config::framerate, true);

	return 0;
}