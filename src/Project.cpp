#include "StdAfx.hpp"
#include "Project.hpp"
#include "Window.hpp"
#include "ModelManager.hpp"
#include "Music.hpp"
#include "Image.hpp"

Project* Project::instance = nullptr;

Project::Project()
	: Resource("project.bin"),
	m_version(0)
{
	startLoad();
}

void Project::onLoad(BinaryReader reader)
{
	reader >> m_version;

	ModelManager::loadProject(reader);
	Music::loadProject(reader);
	ImageManager::loadProject(reader);

	Window::onProjectLoad(true);
}

string Project::text(const string& textId) const
{
	return textId;
}