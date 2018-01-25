#include "StdAfx.hpp"
#include "WndLogin.hpp"

WndLogin::WndLogin()
	: WndWindow("ui.login.title")
{
	WndBoxContainer* container = new WndBoxContainer(Wnd::Vertical);

	WndButton* btnConnect = new WndButton("ui.login.btn_connect");
	container->insert(btnConnect);

	WndButton* btnAbout = new WndButton("ui.login.btn_connect");
	container->insert(btnAbout);

	WndButton* btnQuit = new WndButton("ui.login.btn_connect");
	container->insert(btnQuit);

	setCentralControl(container);
}