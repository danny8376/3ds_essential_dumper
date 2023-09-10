#pragma once

#include "common.h"

#define ARM9_DEBUG_PREFIX "[DEBUG] "

#define debugPrintf(format, args...) logDebugPrintf(format, args)

void debugPrint(const char* str);

// Hand-rolled minimal printf
void PRINTF_ARGS(1) logDebugPrintf(const char* format, ...);
