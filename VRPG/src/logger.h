#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
typedef __int64 lUInt64;
#else
typedef long long lUInt64;
#endif

lUInt64 GetCurrentTimeMillis();

/// Logger
class CRLog
{
public:
	/// log levels
	enum log_level {
		LL_FATAL,
		LL_ERROR,
		LL_WARN,
		LL_INFO,
		LL_DEBUG,
		LL_TRACE
	};
	/// set current log level
	static void setLogLevel(log_level level);
	/// returns current log level
	static log_level getLogLevel();
	/// returns true if specified log level is enabled
	static bool isLogLevelEnabled(log_level level);
	/// returns true if log level is DEBUG or lower
	static bool inline isDebugEnabled() { return isLogLevelEnabled(LL_DEBUG); }
	/// returns true if log level is TRACE
	static bool inline isTraceEnabled() { return isLogLevelEnabled(LL_TRACE); }
	/// returns true if log level is INFO or lower
	static bool inline isInfoEnabled() { return isLogLevelEnabled(LL_INFO); }
	/// returns true if log level is WARN or lower
	static bool inline isWarnEnabled() { return isLogLevelEnabled(LL_WARN); }
	static void fatal(const char * msg, ...);
	static void error(const char * msg, ...);
	static void warn(const char * msg, ...);
	static void info(const char * msg, ...);
	static void debug(const char * msg, ...);
	static void trace(const char * msg, ...);
	/// sets logger instance
	static void setLogger(CRLog * logger);
	virtual ~CRLog();

	/// write log to specified file, flush after every message if autoFlush parameter is true
	static void setFileLogger(const char * fname, bool autoFlush = false);
	/// use stdout for output
	static void setStdoutLogger();
	/// use stderr for output
	static void setStderrLogger();
protected:
	CRLog();
	virtual void log(const char * level, const char * msg, va_list args) = 0;
	log_level curr_level;
	static CRLog * CRLOG;
};

#endif //LOGGER_H_INCLUDED