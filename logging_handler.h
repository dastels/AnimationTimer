// Abstract logging handler
//
// Copyright (c) 2020 Dave Astels

#ifndef __LOGGING_HANDLER_H__
#define __LOGGING_HANDLER_H__


class LoggingHandler
{
private:
  char _buffer[256];

protected:
  bool _initialized;

public:
  LoggingHandler();
  char *format(const char *level_name, const char *msg);
  virtual void emit(const char *level_name, const char *msg) = 0;
  virtual bool initialized(void) { return _initialized; }
  virtual const char* name(void) = 0;
  virtual void clear(void) {};
};


#endif
