#include "StdAfx.hpp"
#include "File.hpp"
#include "FileNode.hpp"

#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#pragma warning(disable: 4334)

#include "miniz.hpp"

const FileNode FileNode::NullNode;

namespace File
{
	namespace
	{
		struct Package
		{
			const char* name;
			bool used;
		};

		vector<char> s_mapStringBuffer;

		vector<pair<const char*, const char*>> s_map;

		vector<Package> s_packages;

		void onPackageLoad(void* arg, void* compressedBuffer, int size)
		{
			const char* packageName = (const char*)arg;
			const Package* pack = nullptr;

			for (size_t i = 0; i < s_packages.size(); i++)
			{
				if (packageName == s_packages[i].name)
				{
					pack = &s_packages[i];
					break;
				}
			}

			if (!pack)
				return;

			const char* curCompressed = (const char*)compressedBuffer;

			if (size < 12 || memcmp(curCompressed, "%CJS", 4) != 0)
			{
				emscripten_log(EM_LOG_ERROR, "Invalid package file '%s'", packageName);
				emscripten_cancel_main_loop();
				return;
			}

			curCompressed += 4;
			uint32_t uncompressedDataSize, compressedDataSize;
			memcpy(&uncompressedDataSize, curCompressed, 4); curCompressed += 4;
			memcpy(&compressedDataSize, curCompressed, 4); curCompressed += 4;

			char* uncompressedBuffer = new char[uncompressedDataSize];
			mz_ulong uncompressedTemp;

			mz_uncompress((unsigned char*)uncompressedBuffer, &uncompressedTemp, (const unsigned char*)curCompressed, (mz_ulong)compressedDataSize);

			uint32_t valuesIndex;
			uint32_t valuesCount;
			uint32_t objectIndex;
			uint32_t objectCount;
			uint32_t rawDataIndex;
			uint32_t rawDataCount;
			uint32_t stringDataIndex;
			uint32_t stringDataCount;

			memcpy(&valuesIndex, uncompressedBuffer + 4 * 0, 4);
			memcpy(&valuesCount, uncompressedBuffer + 4 * 1, 4);
			memcpy(&objectIndex, uncompressedBuffer + 4 * 2, 4);
			memcpy(&objectCount, uncompressedBuffer + 4 * 3, 4);
			memcpy(&rawDataIndex, uncompressedBuffer + 4 * 4, 4);
			memcpy(&rawDataCount, uncompressedBuffer + 4 * 5, 4);
			memcpy(&stringDataIndex, uncompressedBuffer + 4 * 6, 4);
			memcpy(&stringDataCount, uncompressedBuffer + 4 * 7, 4);

			struct FileNodeRaw
			{
				uint32_t type : 3;
				uint32_t size : 29;
				uint32_t val : 32;
			};

			struct ObjectPairRaw
			{
				uint32_t key;
				uint32_t value;
			};

			FileNodeRaw* values = new FileNodeRaw[valuesCount];
			ObjectPairRaw* objectPairs = new ObjectPairRaw[objectCount];
			uint64_t* rawDataBlocks = new uint64_t[rawDataCount];
			char* stringData = new char[stringDataCount];

			memcpy(values, uncompressedBuffer + valuesIndex, sizeof(FileNodeRaw) * valuesCount);
			memcpy(objectPairs, uncompressedBuffer + objectIndex, sizeof(ObjectPairRaw) * objectCount);
			memcpy(rawDataBlocks, uncompressedBuffer + rawDataIndex, sizeof(uint64_t) * rawDataCount);
			memcpy(stringData, uncompressedBuffer + stringDataIndex, stringDataCount);

			FileNode* nodes = new FileNode[valuesCount];
			FileNode::Pair* pairs = new FileNode::Pair[objectCount];

			for (uint32_t i = 0; i < objectCount; i++)
			{
				FileNode::Pair& pair = pairs[i];
				const ObjectPairRaw& rawPair = objectPairs[i];

				pair.k = stringData + rawPair.key;
				pair.v = nodes + rawPair.value;
			}

			for (uint32_t i = 0; i < valuesCount; i++)
			{
				const FileNodeRaw& rawNode = values[i];
				const FileNode::Type type = (FileNode::Type)rawNode.type;

				switch (type)
				{
				case FileNode::Int:
				case FileNode::UInt:
				case FileNode::Float:
					nodes[i] = FileNode(type, rawNode.val);
					break;
				case FileNode::String:
					nodes[i] = FileNode(type, rawNode.size, stringData + rawNode.val);
					break;
				case FileNode::Object:
					nodes[i] = FileNode(type, rawNode.size, pairs + rawNode.val);
					break;
				case FileNode::Array:
					nodes[i] = FileNode(type, rawNode.size, nodes + rawNode.val);
					break;
				case FileNode::RawData:
					nodes[i] = FileNode(type, rawNode.size, rawDataBlocks + rawNode.val);
					break;
				}
			}

			delete[] uncompressedBuffer;
			delete[] values;
			delete[] objectPairs;

			File::LoadData loadData;

			for (auto it = nodes->begin(); it != nodes->end(); it++)
			{
				loadData.node = it.value();

				loadData.baseName = it.key();
				loadData.ext = loadData.baseName.substr(loadData.baseName.find_last_of('.') + 1);
				loadData.dir = loadData.baseName.substr(0, loadData.baseName.find_last_of('/'));
				loadData.baseName = loadData.baseName.substr(loadData.dir.size() + 1, loadData.baseName.size() - loadData.dir.size() - loadData.ext.size() - 2);

				loadData.freeCallback = nullptr;

				FileLoader::handle(loadData);
			}

			delete[] pairs;
			delete[] nodes;
			delete[] rawDataBlocks;
			delete[] stringData;
		}

		void onPackageLoadError(void* arg)
		{
			const char* name = (const char*)arg;

			char buffer[512];
			sprintf(buffer, "./%s.bin", name);
			emscripten_log(EM_LOG_ERROR, "Failed to load package '%s'", buffer);
		}

		void loadPackage(const char* name)
		{
			for (size_t i = 0; i < s_packages.size(); i++)
			{
				if (strcmp(name, s_packages[i].name) == 0)
				{
					s_packages[i].used = true;
					return;
				}
			}

			Package pak;
			pak.name = name;
			pak.used = true;
			s_packages.push_back(pak);

			char buffer[512];
			sprintf(buffer, "./%s.bin", name);
			emscripten_async_wget_data(buffer, (void*)name, onPackageLoad, onPackageLoadError);
		}

		void onFilemapLoad(void* arg, void* compressedBuffer, int size)
		{
			const char* curCompressed = (const char*)compressedBuffer;

			if (size < 12 || memcmp(curCompressed, "%MAP", 4) != 0)
			{
				emscripten_log(EM_LOG_ERROR, "Invalid filemap data");
				emscripten_cancel_main_loop();
				return;
			}

			curCompressed += 4;
			uint32_t uncompressedDataSize, compressedDataSize;
			memcpy(&uncompressedDataSize, curCompressed, 4); curCompressed += 4;
			memcpy(&compressedDataSize, curCompressed, 4); curCompressed += 4;

			char* uncompressedBuffer = new char[uncompressedDataSize];
			mz_ulong uncompressedTemp;

			mz_uncompress((unsigned char*)uncompressedBuffer, &uncompressedTemp, (const unsigned char*)curCompressed, (mz_ulong)compressedDataSize);

			char* curUncompressed = uncompressedBuffer;

			uint32_t stringDataSize, fileCount;
			memcpy(&stringDataSize, curUncompressed, 4); curUncompressed += 4;
			memcpy(&fileCount, curUncompressed, 4); curUncompressed += 4;

			s_mapStringBuffer.resize(stringDataSize);
			memcpy(s_mapStringBuffer.data(), curUncompressed, stringDataSize); curUncompressed += stringDataSize;

			s_map.resize(fileCount);

			uint32_t key, value;

			for (uint32_t i = 0; i < fileCount; i++)
			{
				memcpy(&key, curUncompressed, 4); curUncompressed += 4;
				memcpy(&value, curUncompressed, 4); curUncompressed += 4;

				s_map[i] = pair<const char*, const char*>(s_mapStringBuffer.data() + key, s_mapStringBuffer.data() + value);
			}

			delete[] uncompressedBuffer;

			loadPackage(FILE_START_PACKAGE);
		}

		void onFilemapLoadError(void* arg)
		{
			emscripten_log(EM_LOG_ERROR, "Failed to load filemap");
			emscripten_cancel_main_loop();
		}
	}

	void initialize()
	{
		emscripten_async_wget_data("./filemap.bin", 0, onFilemapLoad, onFilemapLoadError);
	}

	void load(const char* filepath)
	{
		const char* packageName = nullptr;

		for (size_t i = 0; i < s_map.size(); i++)
		{
			if (strcmp(filepath, s_map[i].first) == 0)
			{
				packageName = s_map[i].second;
				break;
			}
		}

		if (!packageName)
			emscripten_log(EM_LOG_ERROR, "File '%s' doesn't exist", filepath);
		else
			loadPackage(packageName);
	}

	void markPackagesUnused()
	{
		for (size_t i = 0; i < s_packages.size(); i++)
			s_packages[i].used = false;
	}

	void freeUnusedPackages()
	{
		for (auto it = s_packages.begin(); it != s_packages.end();)
		{
			if (it->used)
				it++;
			else
				it = s_packages.erase(it);
		}
	}
}