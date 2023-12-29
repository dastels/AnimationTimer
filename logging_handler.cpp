// Abstract logging handler
//
// Copyright (c) 2020 Dave Astels

#include <Arduino.h>
#include "logging_handler.h"


LoggingHandler::LoggingHandler()
  : _initialized(true)
{
}

char *LoggingHandler::format(const char *level_name, const char *msg)
{
  sprintf(_buffer, "%9lu - %8s: %s", millis(), level_name, msg);
  return _buffer;
}
