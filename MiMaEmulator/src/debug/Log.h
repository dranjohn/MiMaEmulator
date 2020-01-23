#pragma once

//std library
#include <memory>

//external vendor libraries
#include <spdlog/spdlog.h>


namespace MiMa {
	constexpr spdlog::level::level_enum STANDARD_SPDLOG_DEFAULT_LEVEL = spdlog::level::level_enum::trace;
	constexpr char* STANDARD_SPDLOG_STDOUT_PATTERN = "%^[%T] %n: %v%$";
	constexpr char* STANDARD_SPDLOG_FILE_PATTERN = "[%L| %T] %n: %v";

	class Logger {
	private:
		std::shared_ptr<spdlog::logger> spdlogLogger;
	public:
		Logger(char* name, spdlog::level::level_enum logLevel = STANDARD_SPDLOG_DEFAULT_LEVEL, const char* loggerPattern = STANDARD_SPDLOG_STDOUT_PATTERN);
		Logger(std::shared_ptr<spdlog::logger>& spdlogLogger);

		inline std::shared_ptr<spdlog::logger> get_spdlogLogger() { return spdlogLogger; };
		inline void setLogLevel(spdlog::level::level_enum logLevel) { spdlogLogger->set_level(logLevel); };
	};

	extern Logger mimaDefaultLog;
}


#ifdef MIMA_DEBUG
#define MIMA_LOG
#endif //MIMA_DEBUG

#ifdef MIMA_LOG

#define MIMA_LOG_TRACE(...) MiMa::mimaDefaultLog.get_spdlogLogger()->trace(__VA_ARGS__)
#define MIMA_LOG_INFO(...) MiMa::mimaDefaultLog.get_spdlogLogger()->info(__VA_ARGS__)
#define MIMA_LOG_WARN(...) MiMa::mimaDefaultLog.get_spdlogLogger()->warn(__VA_ARGS__)
#define MIMA_LOG_ERROR(...) MiMa::mimaDefaultLog.get_spdlogLogger()->error(__VA_ARGS__)
#define MIMA_LOG_CRITICAL(...) MiMa::mimaDefaultLog.get_spdlogLogger()->critical(__VA_ARGS__)

#define MIMA_ASSERT_TRACE(assertion, ...) if (!(assertion)) MiMa::mimaDefaultLog.get_spdlogLogger()->trace(__VA_ARGS__)
#define MIMA_ASSERT_INFO(assertion, ...) if (!(assertion)) MiMa::mimaDefaultLog.get_spdlogLogger()->info(__VA_ARGS__)
#define MIMA_ASSERT_WARN(assertion, ...) if (!(assertion)) MiMa::mimaDefaultLog.get_spdlogLogger()->warn(__VA_ARGS__)
#define MIMA_ASSERT_ERROR(assertion, ...) if (!(assertion)) MiMa::mimaDefaultLog.get_spdlogLogger()->error(__VA_ARGS__)
#define MIMA_ASSERT_CRITICAL(assertion, ...) if (!(assertion)) MiMa::mimaDefaultLog.get_spdlogLogger()->critical(__VA_ARGS__)

#else

#define MIMA_LOG_TRACE(...)
#define MIMA_LOG_INFO(...)
#define MIMA_LOG_WARN(...)
#define MIMA_LOG_ERROR(...)
#define MIMA_LOG_CRITICAL(...)

#define MIMA_ASSERT_TRACE(assertion, ...)
#define MIMA_ASSERT_INFO(assertion, ...)
#define MIMA_ASSERT_WARN(assertion, ...)
#define MIMA_ASSERT_ERROR(assertion, ...)
#define MIMA_ASSERT_CRITICAL(assertion, ...)

#endif
