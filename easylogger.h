//! \mainpage Easylogger
//!
//! easylogger - Simple "good enough" C++ logging framework
//! 
//! Copyright (C) 2010 Sean Middleditch <sean@middleditch.us>
//!
//! Easylogger is free software; you can redistribute it and/or modify
//! it under the terms of the MIT license. See LICENSE for details.

#if !defined(EASYLOGGER_H)
#define EASYLOGGER_H

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

//! Main namespace containing all Easylogger functionality
namespace easylogger {

	class Logger;

	//! Log levels
	enum LogLevel {
		LEVEL_TRACE,	//!< Trace-level messages (0)
		LEVEL_DEBUG,	//!< Debug-level messages (1)
		LEVEL_INFO,		//!< Info-level messages (2)
		LEVEL_WARNING,	//!< Warning-level messages (3)
		LEVEL_ERROR,	//!< Error-level message (4)
		LEVEL_FATAL		//!< Fatal-level message (5)
	};

	//! Private namespace
	//! \internal
	namespace _private {

		//! Sink for log message streaming
		//!
		//! \internal
		class LogSink {
		public:
			//! Construct a new sink
			//!
			//! \param logger Logger instance to log to.
			//! \param level Level of messages generated by sink.
			//! \param file File name of log location.
			//! \param line Line of file of log location.
			//! \param func Name of function at log location.
			LogSink(Logger* logger, LogLevel level, const char* file,
					unsigned int line, const char* func) : _logger(logger),
					_level(level), _file(file), _line(line), _func(func) {}

			//! Copy constructor
			//!
			//! \param sink Source LogSink.
			LogSink(const LogSink& sink) : _logger(sink._logger),
					_level(sink._level), _file(sink._file), _line(sink._line),
					_func(sink._func) {}

			//! Get the internal stream of the sink
			//!
			//! \returns internal std::ostream
			::std::ostream& Stream() { return _os; }

			inline ~LogSink();
			
		private:
			::std::ostringstream _os;

			Logger* _logger;

			LogLevel _level;

			const char* _file;

			unsigned int _line;

			const char* _func;
		};

		//! Tracer that handles exits at end of scope
		//!
		//! \internal
		class Tracer {
		public:
			//! Construct a new Tracer instance.
			//!
			//! This is meant to be used by the EASY_TRACE macro.
			//!
			//! \param logger Logger instance to log trace to.
			//! \param file Name of file at trace point.
			//! \param line Line number of file at trace point.
			//! \param func Name of function at trace point.
			//! \param name Name of trace point.
			inline Tracer(Logger& logger, const char* file, unsigned int line,
					const char* func, const char* name);

			inline ~Tracer();

		private:
			Logger& _logger;

			const char* _file;

			const char* _func;

			const char* _name;
		};

	} // namespace _private

	//! Logger system core class
	class Logger {
	public:
		//! Construct a new Logger
		//!
		//! \param name Name of logger used in log messages.
		Logger(const ::std::string& name) : _name(name), _parent(0),
				_level(LEVEL_INFO), _stream(&::std::cout),
				_format("[%F:%C %P] %N %L: %S") {}

		//! Construct a new Logger with a parent
		//!
		//! \param name Name of logger used in log messages.
		//! \param parent Parent Logger all messages are forwarded to.
		Logger(const ::std::string& name, Logger& parent) : _name(name),
				_parent(&parent), _level(LEVEL_INFO), _stream(0),
				_format("[%F:%C %P] %N %L: %S") {}

		~Logger() {}

		//! Get the name of the Logger
		//!
		//! \returns Logger's name
		const ::std::string& Name() const { return _name; }

		//! Get the minimum log level of the Logger
		//!
		//! \returns Minimum log level
		LogLevel Level() const { return _level; }

		//! Set the minimum log level of the Logger
		//!
		//! \returns New minimum log level
		LogLevel Level(LogLevel level) { return _level = level; }

		//! Checks if this Logger or any ancestor accepts a given log level
		//!
		//! Each Logger will only log messages of a particular level or
		//! higher.  This method will check if this Logger instance or
		//! any of its parents are willing to log a message of a given
		//! log level.
		//!
		//! \param level Log level to check for.
		//! \returns true if any ancestor will accept log level
		inline bool IsLevel(LogLevel level) const;

		//! Create a new log sink
		//!
		//! Does the actual work of writing log message.
		//!
		//! \param level Level of log message.
		//! \param file Name of file at point of log.
		//! \param line Line of file at point of log.
		//! \param func Name of function at point of log.
		inline _private::LogSink Log(LogLevel level, const char* file,
				unsigned int line, const char* func);

		//! Get the underlying stream
		//!
		//! \returns underlying stream
		::std::ostream& Stream() const { return *_stream; }

		//! Set the underlying stream
		//!
		//! Note that this does not copy or otherwise ensure that the
		//! given stream will persist.  If you pass in a stream to this
		//! method, you are responsible for ensuring that the stream is
		//! not destructed before the Logger is.
		//!
		//! \param stream New underlying stream.
		//! \returns New underlying stream.
		inline ::std::ostream& Stream(::std::ostream& stream);

		//! Get the log format string
		//!
		//! \returns Log format string.
		const ::std::string& Format() const { return _format; }

		//! Set the log format string
		//!
		//! \param format New log format string.
		//! \returns Log format string.
		const ::std::string& Format(const ::std::string& format);

		//! Flushes underlying output stream
		void Flush() { _stream->flush(); }
	
	private:
		//! Write log to stream
		//!
		//! Does the actual work of writing log message.
		//!
		//! \param level Level of log message.
		//! \param logger Original Logger target of message.
		//! \param file Name of file at point of log.
		//! \param line Line of file at point of log.
		//! \param func Name of function at point of log.
		//! \param message The log message.
		void WriteLog(LogLevel level, Logger* logger, const char* file,
				unsigned int line, const char* func, const char* message);

		::std::string _name;

		Logger* _parent;

		LogLevel _level;

		::std::ostream* _stream;

		::std::string _format;

		friend class _private::LogSink;
	};

} // namespace easylogger

//! Stream operator for LogSink
template <typename T>
::easylogger::_private::LogSink& operator<<(::easylogger::_private::LogSink& sink, const T& val) {
	sink.Stream() << val;
	return sink;
}

//! General logging helper
//!
//! \internal
//! \param logger Logger to log to.
//! \param level Level to log at
//! \param message Stream message.
#define EASY_LOG(logger, level, message) do{ \
		if ((logger).IsLevel((level))) { \
			do { \
				::easylogger::_private::LogSink _easy_sink((logger).Log(level, __FILE__, __LINE__, __FUNCTION__)); \
				_easy_sink << message; \
			} while(0); \
			if ((level) == ::easylogger::LEVEL_FATAL) { \
				(logger).Flush(); \
				std::abort(); \
			} \
		} \
	}while(0)

#define LOG_TRACE(logger, message) EASY_LOG((logger), ::easylogger::LEVEL_TRACE, message)
#define LOG_DEBUG(logger, message) EASY_LOG((logger), ::easylogger::LEVEL_DEBUG, message)
#define LOG_INFO(logger, message) EASY_LOG((logger), ::easylogger::LEVEL_INFO, message)
#define LOG_WARNING(logger, message) EASY_LOG((logger), ::easylogger::LEVEL_WARNING, message)
#define LOG_ERROR(logger, message) EASY_LOG((logger), ::easylogger::LEVEL_ERROR, message)
#define LOG_FATAL(logger, message) EASY_LOG((logger), ::easylogger::LEVEL_FATAL, message)


#if !defined(NDEBUG)
# define EASY_ASSERT(logger, expr, msg) do{ \
		if (!(expr)) { \
			EASY_LOG((logger), ::easylogger::LEVEL_FATAL, "ASSERTION FAILED: " #expr ": " msg); \
		} \
	}while(0)
#else
# define EASY_ASSERT(logger, expr, msg) do{ }while(0)
#endif

#define ASSERT_NOTNULL(logger, expr, msg) EASY_ASSERT((logger), (expr) != NULL, msg)
#define ASSERT_EQ(logger, lhs, rhs, msg) EASY_ASSERT((logger), (lhs) == (rhs), msg)
#define ASSERT_NE(logger, lhs, rhs, msg) EASY_ASSERT((logger), (lhs) != (rhs), msg)
#define ASSERT_TRUE(logger, expr) EASY_ASSERT((logger), (lhs) == true, msg)
#define ASSERT_FALSE(logger, expr, msg) EASY_ASSERT((logger), (lhs) != false, msg)

#define EASY_TRACE(logger, name) ::easylogger::_private::Tracer easy_trace_ ## name((logger), __FILE__, __LINE__, __FUNCTION__, #name)

#include "easylogger-impl.h"

#endif
