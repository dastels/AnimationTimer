// Serial logging handler (mainly for debug mode)
//
// Copyright (c) 2020 Dave Astels

#include <Arduino.h>
#include "serial_handler.h"


SerialHandler::SerialHandler(): LoggingHandler()
{
  Serial.begin(115200);
  while (!Serial);
}


void SerialHandler::emit(const char *level_name, const char *msg)
{
  Serial.println(format(level_name, msg));
  Serial.flush();
}
