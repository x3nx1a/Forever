#include "StdAfx.hpp"
#include "ModelFile.hpp"
#include "Object3D.hpp"
#include "SfxBase.hpp"

namespace
{
	enum class ComponentType
	{
		None,
		Object3D,
		Sfx,
		Motion,
		Skeleton
	};
}

ModelFile::ModelFile(const string& filename)
	: Resource(filename),
	m_obj(nullptr),
	m_skel(nullptr),
	m_sfx(nullptr)
{
	startLoad();
}

ModelFile::~ModelFile()
{
	if (m_obj)
		delete m_obj;
	if (m_skel)
		delete m_skel;
	for (std::size_t i = 0; i < m_motions.size(); i++)
		delete m_motions[i];
	if (m_sfx)
		delete m_sfx;
}

void ModelFile::onLoad(BinaryReader reader)
{
	const uint8_t ver = reader.read<uint8_t>();

	uint8_t type, nameLen;
	char name[32];

	do
	{
		reader >> type
			>> nameLen;

		if (nameLen)
			reader.read(name, nameLen);
		name[nameLen] = '\0';

		switch ((ComponentType)type)
		{
		case ComponentType::Object3D:
			m_obj = new Object3D();
			m_obj->load(reader, ver);
			break;
		case ComponentType::Skeleton:
			m_skel = new Skeleton();
			m_skel->load(reader, ver);
			break;
		case ComponentType::Sfx:
			m_sfx = new SfxBase();
			m_sfx->load(reader, ver);
			break;
		case ComponentType::Motion:
		{
			Motion* motion = new Motion(name);
			motion->load(reader, ver);
			m_motions.push_back(motion);
			break;
		}
		default:
			break;
		}

	} while (type != (uint8_t)ComponentType::None);

	m_motions.shrink_to_fit();
}