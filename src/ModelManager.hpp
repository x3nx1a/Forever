#pragma once

#include "BinaryReader.hpp"
#include "Model.hpp"
#include "ModelFile.hpp"

namespace ModelManager
{
	ModelPtr createModel(ObjectType objType, int id);

	const ModelProp* modelProp(ObjectType type, int id);

	ModelFilePtr getModelFile(const string& filename);

	void loadProject(BinaryReader& reader);
}