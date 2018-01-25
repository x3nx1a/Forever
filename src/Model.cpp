#include "StdAfx.hpp"
#include "Model.hpp"

Model::Model(const ModelProp* prop)
	: m_prop(prop),
	m_loaded(false),
	m_frameCount(0),
	m_currentFrame(0.0f),
	m_endFrame(false)
{
}