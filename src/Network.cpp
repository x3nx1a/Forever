#include "StdAfx.hpp"
#include "Network.hpp"

namespace Network
{
	const string& resourcesPath()
	{
		static string resPath = "./";
		return resPath;
	}
}