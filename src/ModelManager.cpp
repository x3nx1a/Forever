#include "StdAfx.hpp"
#include "ModelManager.hpp"
#include "Mesh.hpp"
#include "SfxModel.hpp"

#include <unordered_map>

namespace ModelManager
{
	namespace
	{
		vector<ModelProp> s_modelProps[MAX_OBJTYPE];

		vector<ModelPtr> s_meshes;

		unordered_map<string, ModelFilePtr> s_modelFiles;
	}

	ModelPtr createModel(ObjectType objType, int id)
	{
		const ModelProp* prop = modelProp(objType, id);
		if (!prop)
			return nullptr;

		switch (prop->modelType)
		{
		case MODELTYPE_MESH:
		{
			for (std::size_t i = 0; i < s_meshes.size(); i++)
				if (s_meshes[i]->prop() == prop)
					return s_meshes[i];

			ModelPtr meshPtr = ModelPtr::create<Mesh>(prop);
			((Mesh*)meshPtr.get())->loadPart(prop->filename);
			s_meshes.push_back(meshPtr);
			return meshPtr;
		}
		case MODELTYPE_ANIMATED_MESH:
		{
			ModelPtr meshPtr = ModelPtr::create<Mesh>(prop);
			((Mesh*)meshPtr.get())->loadPart(prop->filename);
			return meshPtr;
		}
		case MODELTYPE_SFX:
		{
			ModelPtr sfxPtr = ModelPtr::create<SfxModel>(prop);
			((SfxModel*)sfxPtr.get())->load(prop->filename);
			return sfxPtr;
		}
		default:
			return nullptr;
		}
	}

	void loadProject(BinaryReader& reader)
	{
		uint8_t objType;
		int modelCount, i, j, len;

		for (i = 0; i < 2; i++)
		{
			reader >> objType
				>> modelCount;

			s_modelProps[objType].resize(modelCount);

			for (j = 0; j < modelCount; j++)
			{
				ModelProp& prop = s_modelProps[objType][j];

				reader >> prop.id
					>> len;
				reader.read(prop.filename, len);
				prop.filename[len] = '\0';
				reader >> prop.modelType
					>> prop.distant;
			}
		}
	}

	const ModelProp* modelProp(ObjectType type, int id)
	{
		const vector<ModelProp>& models = s_modelProps[type];
		for (std::size_t i = 0; i < models.size(); i++)
			if (models[i].id == id)
				return &models[i];

		emscripten_log(EM_LOG_ERROR, "Model prop %d (type %d) not found", id, type);
		return nullptr;
	}

	ModelFilePtr getModelFile(const string& filename)
	{
		auto it = s_modelFiles.find(filename);
		if (it != s_modelFiles.end())
			return it->second;

		ModelFilePtr model = ModelFilePtr::create("model/" + filename + ".bin");
		s_modelFiles.insert(pair<string, ModelFilePtr>(filename, model));
		return model;
	}
}