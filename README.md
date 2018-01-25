# Forever

## Overview

Forever is a test to see if WebGL and ASMJS can run a quite old 3D MMORPG like FlyForFun.

## Live demo

You can try the engine at: http://ffe.aishiro.com/test

Currently only works on Desktop PC (Windows, Linux, Mac) because it only provides DXT compressed textures.
Right click to rotate the camera and use the mouse wheel to move forward and backward.

## Warning

This is an old version of the project, the online source code isn't complete and can't compile easily.

## New version

I'm currently working on a totally new version of the project, divided in two parts:

### Luiic Engine
Luiic engine is an in-house made game engine. Main features are:
* Written in modern C++14 with GLES2, OpenAL and Emscripten API
* Highly-optimized to run on low-performance and low-memory devices
* Works on major platforms: Windows, Linux, Mac (using Qt and ANGLE), iOS and Android (using Qt and GLES2), HTML5 (using Emscripten, ASMJS/WASM and WebGL)
* UI and textures support DPI scaling
* The 3D engine automatically adapts graphics quality settings to device capacity
* Resources are in an unique file format, a sort of compiled and compressed JSON with additionnal features, like raw bytes blocks type
* Multiple resources are packed into resource packages in order to optimize HTTP network usage
* Music resources are automatically compiled into several formats (ogg, mp3, webm) and the device downloads its preferred supported format
* Texture resources are automatically compiled into several formats (DXT1/3/5, ETC2, PVRTC, RGBA8) and the device downloads its preferred supported format

### FlyForEver
ForForEver is the Luiic engine used with this good old FlyForFun online PC game.

### More informations soon!
