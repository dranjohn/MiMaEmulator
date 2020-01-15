#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace MiMa {
	Logger::Logger(char* name, spdlog::level::level_enum logLevel, const char* loggerPattern) {
		spdlogLogger = spdlog::stdout_color_mt(name);
		spdlogLogger->set_level(logLevel);
		spdlogLogger->set_pattern(loggerPattern);
	}

	Logger::Logger(std::shared_ptr<spdlog::logger>& spdlogLogger) : spdlogLogger(spdlogLogger) {}
	

	Logger constructDefaultLogger() {
		//create stdout sink with default level info
		std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		stdoutSink->set_level(spdlog::level::level_enum::info);
		stdoutSink->set_pattern(STANDARD_SPDLOG_STDOUT_PATTERN);

		//create file sink with default level trace
#ifdef MIMA_LOG
		std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log/mimaEmulation.log", true);
		fileSink->set_level(spdlog::level::level_enum::trace);
		fileSink->set_pattern(STANDARD_SPDLOG_FILE_PATTERN);
#endif

		//create spdlog logger
		spdlog::sinks_init_list sinks = { stdoutSink,
#ifdef MIMA_LOG
			fileSink
#endif
		};

		std::shared_ptr<spdlog::logger> spdlogLogger = std::make_shared<spdlog::logger>("mimaDefaultLogger", sinks);
		spdlogLogger->set_level(spdlog::level::level_enum::trace);

		return Logger(spdlogLogger);
	}


	Logger mimaDefaultLog = constructDefaultLogger();
}
