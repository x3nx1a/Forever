/*
#include "StdAfx.hpp"
#include "Window.hpp"
#include "Shaders.hpp"
#include "GameTime.hpp"
#include "Project.hpp"
#include "Platform.hpp"
#include "Render2D.hpp"
#include "Config.hpp"
#include "Music.hpp"
#include "WndManager.hpp"
#include "WndTheme.hpp"

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
}

const time_t nowSec = time(nullptr);
if (nowSec > s_lastUpdateSec)
{
const int secCount = (int)(nowSec - s_lastUpdateSec);

Music::updateSec(secCount);

s_lastUpdateSec = nowSec;
}

if (s_active && gl::isContextActive())
WndManager::instance->rootRender();
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
Render2D::updateViewport();

if (WndManager::instance)
WndManager::instance->setFixedSize(s_display.size);

return true;
}

EM_BOOL onVisibilityChange(int eventType, const EmscriptenVisibilityChangeEvent* visibilityChangeEvent, void* userData)
{
s_active = (visibilityChangeEvent->hidden == 0);
return true;
}

EM_BOOL onMouseMove(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
if (gl::isContextActive())
WndManager::instance->rootMouseMove(ivec2(mouseEvent->canvasX, mouseEvent->canvasY));
return true;
}

EM_BOOL onMouseDown(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
if (gl::isContextActive())
{
if (mouseEvent->button == 0)
WndManager::instance->rootMouseDown(Wnd::ButtonLeft);
else if (mouseEvent->button == 1)
WndManager::instance->rootMouseDown(Wnd::ButtonMiddle);
else if (mouseEvent->button == 2)
WndManager::instance->rootMouseDown(Wnd::ButtonRight);
}
return true;
}

EM_BOOL onMouseUp(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
if (gl::isContextActive())
{
if (mouseEvent->button == 0)
WndManager::instance->rootMouseUp(Wnd::ButtonLeft);
else if (mouseEvent->button == 1)
WndManager::instance->rootMouseUp(Wnd::ButtonMiddle);
else if (mouseEvent->button == 2)
WndManager::instance->rootMouseUp(Wnd::ButtonRight);
}
return true;
}

EM_BOOL onMouseWheel(int eventType, const EmscriptenWheelEvent* wheelEvent, void* userData)
{
if (!gl::isContextActive())
return true;

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

if (gl::isContextActive())
onContextLost(EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST, 0, 0);

if (WndManager::instance)
{
delete WndManager::instance;
WndManager::instance = nullptr;
}
if (WndTheme::instance)
{
delete WndTheme::instance;
WndTheme::instance = nullptr;
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

WndTheme::instance = WndTheme::create("v19");
WndManager::instance = new WndManager();

onResize(EMSCRIPTEN_EVENT_RESIZE, 0, 0);

s_lastUpdate = emscripten_get_now();
s_lastUpdateSec = time(nullptr);
s_running = true;

WndManager::instance->rootInit();

WndManager::instance->showLoginTitle();
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
*/