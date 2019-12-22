#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace MiMa {
	Logger::Logger(char* name, spdlog::level::level_enum logLevel, const char* loggerPattern) {
		spdlogLogger = spdlog::stdout_color_mt(name);
		spdlogLogger->set_level(logLevel);
		spdlogLogger->set_pattern(loggerPattern);
	}

	Logger mimaDefaultLog("mimaDefaultLogger");
}
