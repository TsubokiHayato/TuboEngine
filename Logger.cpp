#include "Logger.h"
#include <debugapi.h>

namespace Logger {


	//logì¬
	void Log(const std::string& message) {
		OutputDebugStringA(message.c_str());

	}

}
