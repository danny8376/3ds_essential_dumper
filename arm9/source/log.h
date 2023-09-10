#pragma once

#include "common.h"
#include <stdarg.h>

#define LOG_FILE "dumper.log"

bool logReady();
bool initLog();
void deinitLog();

bool logWrite(const void* data, unsigned int btw);
bool logWriteStr(const char* str);
void logWritevf(const char* format, va_list args);
void logWritef(const char* format, ...);
