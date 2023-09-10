#include "debug.h"
#include "smalllib.h"

#include <arm.h>

#ifdef ARM9
#include <log.h>
#else
#include <ui/log.h>
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

void debugPrint(const char* str) {
    if (*str == '\0')
        return;
    //u32 irqState = ARM_EnterCritical();
    logWrite(ARM9_DEBUG_PREFIX, sizeof(ARM9_DEBUG_PREFIX) - 1);
    logWriteStr(str);
    //ARM_LeaveCritical(irqState);
}

void logDebugPrintf(const char* format, ...) {
    //u32 irqState = ARM_EnterCritical();
    va_list args;
    va_start(args, format);

    logWrite(ARM9_DEBUG_PREFIX, sizeof(ARM9_DEBUG_PREFIX) - 1);

    logWritevf(format, args);

    va_end(args);
    //ARM_LeaveCritical(irqState);
}
