// Logger
//
// Copyright (c) 2020 Dave Astels

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "logging.h"

const char *level_names[] = {"NOTSET", "DEBUG_DEEP", "DEBUG_MID", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};


LogLevel log_level_for(const char *level_name)
{
  int name_index = -1;
  for (int i = 0; i < 8; i++) {
    if (strcmp(level_name, level_names[i]) == 0) {
      name_index = i;
      break;
    }
  }
  if (name_index == -1) {
    return LogLevel::NOTSET;
  } else {
    return (LogLevel)(name_index);
  }
}

const char *log_name_for_level(LogLevel level)
{
  switch (level) {
  case LogLevel::NOTSET:
    return "NOTSET";
  case LogLevel::DEBUG_DEEP:
    return "DEBUG_DEEP";
  case LogLevel::DEBUG_MID:
    return "DEBUG_MID";
  case LogLevel::DEBUG:
    return "DEBUG";
  case LogLevel::INFO:
    return "INFO";
  case LogLevel::WARNING:
    return "WARNING";
  case LogLevel::ERROR:
    return "ERROR";
  case LogLevel::CRITICAL:
    return "CRITICAL";
  }
  return "BAD_LEVEL";
}


Logger *Logger::_instance = nullptr;


Logger *Logger::get_logger(LoggingHandler *handler)
{
  if (_instance == nullptr) {
    _instance = new Logger(handler);
  }
  return _instance;
}


Logger *Logger::get_logger(void)
{
  if (_instance == nullptr) {
    _instance = new Logger();
  }
  return _instance;
}


Logger::Logger()
{
  for (int i = 0; i < MAX_NUMBER_OF_HANDLERS; i++) {
    _handlers[i] = nullptr;
  }
}


Logger::Logger(LoggingHandler *handler): Logger()
{
  _handlers[0] = handler;
  for (int i = 1; i < MAX_NUMBER_OF_HANDLERS; i++) {
    _handlers[i] = nullptr;
  }
}


void Logger::set_level(LogLevel new_level)
{
  _level = new_level;
}


// Set a single handler, any existing are removed

void Logger::set_handler(LoggingHandler *handler)
{
  _handlers[0] = handler;
  _handlers[1] = nullptr;
  _handlers[2] = nullptr;
  _handlers[3] = nullptr;
}


// Add a handler
// Return true if it was successful, false if there was no room

bool Logger::add_handler(LoggingHandler *handler)
{
  for (int i = 0; i < MAX_NUMBER_OF_HANDLERS; i++) {
    if (_handlers[i] == nullptr) {
      _handlers[i] = handler;
      return true;
    }
  }
  return false;
}


void Logger::report(void)
{
  char buf[512];
  buf[0] = 0;
  for (int i = 0; i < MAX_NUMBER_OF_HANDLERS; i++) {
    if (_handlers[i]) {
      if (i > 0) {
        strcat(buf, "                                   ");
      }
      strcat(buf, _handlers[i]->name());
      strcat(buf, "\r\n");
    }
  }
  debug("Logger:\r\n                         level: %d\r\n                         max handlers: %d\r\n                         handlers: %s", _level, MAX_NUMBER_OF_HANDLERS, buf);
}


void Logger::_internal_log(LogLevel level, const char *format, va_list args)
{
  if (level >= _level) {
    vsprintf(_buffer, format, args);
    for (int i = 0; i < MAX_NUMBER_OF_HANDLERS; i++) {
      if (_handlers[i]) {
        _handlers[i]->emit(log_name_for_level(level), _buffer);
      }
    }
  }
}


void Logger::log(LogLevel level, const char *format, ...)
{
  va_list args;
  va_start (args, format);
  _internal_log(level, format, args);
  va_end (args);
}


void Logger::debug_deep(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  _internal_log(LogLevel::DEBUG_DEEP, format, args);
  va_end (args);
}


void Logger::debug_mid(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  _internal_log(LogLevel::DEBUG_MID, format, args);
  va_end (args);
}


void Logger::debug(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  _internal_log(LogLevel::DEBUG, format, args);
  va_end (args);
}


void Logger::info(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  _internal_log(LogLevel::INFO, format, args);
  va_end (args);
}


void Logger::warning(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  _internal_log(LogLevel::WARNING, format, args);
  va_end (args);
}


void Logger::error(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  _internal_log(LogLevel::ERROR, format, args);
  va_end (args);
}


void Logger::critical(const char *format, ...)
{
  va_list args;
  va_start (args, format);
  _internal_log(LogLevel::CRITICAL, format, args);
  va_end (args);
}


void Logger::clear(void)
{
  for (int i = 0; i < MAX_NUMBER_OF_HANDLERS; i++) {
    if (_handlers[i]) {
      _handlers[i]->clear();
    }
  }
}
