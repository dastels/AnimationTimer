// -*- mode: c++ -*-
// Logger
//
// The MIT License (MIT)
//
// Copyright (c) 2020 Dave Astels
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef __LOGGING_H__
#define __LOGGING_H__

enum class LogLevel : int { NOTSET, DEBUG_DEEP, DEBUG_MID, DEBUG, INFO, WARNING, ERROR, CRITICAL };

#include <stdarg.h>
#include "logging_handler.h"

#ifndef MAX_NUMBER_OF_HANDLERS
#define MAX_NUMBER_OF_HANDLERS (4)
#endif

class Logger
{
  static Logger *_instance;
  LoggingHandler *_handlers[MAX_NUMBER_OF_HANDLERS];
  LogLevel _level;
  char _buffer[1024];
  void _internal_log(LogLevel level, const char *format, va_list args);

 public:
  Logger();
  Logger(LoggingHandler *handler);
  static Logger *get_logger(LoggingHandler *handler);
  static Logger *get_logger(void);
  LogLevel get_level() { return _level; }
  void set_level(LogLevel new_level);
  void set_handler(LoggingHandler *_handler);
  bool add_handler(LoggingHandler *_handler);
  bool initialized() { return _handlers[0]->initialized(); }
  void report(void);
  void log(LogLevel level, const char *format, ...);
  void debug_deep(const char *format, ...);
  void debug_mid(const char *format, ...);
  void debug(const char *format, ...);
  void info(const char *format, ...);
  void warning(const char *format, ...);
  void error(const char *format, ...);
  void critical(const char *format, ...);
  void clear(void);
};

const char *log_name_level_for(LogLevel level);
LogLevel log_level_for(const char *level_name);


#endif
