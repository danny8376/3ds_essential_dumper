#include "log.h"

#include "smalllib.h"
#include "fatfs/ff.h"
#include "futils.h"

#include <arm.h>

static FATFS fs;
static FIL logfile;
static bool logready = false;

bool logReady() {
    return logready;
}

bool initLog() {
    if (f_mount(&fs, "0:", 1) != FR_OK)
        return false;

    FRESULT res = f_mkdir(MAIN_PATH);
    if (res != FR_OK && res != FR_EXIST)
        return false;

    //if (f_chdir(MAIN_PATH) != FR_OK)
    //    return false;

    char path[MAX_PATH];
    if (f_ropen(&logfile, path, LOG_FILE, FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
        f_unmount("0:");
        return false;
    }

    logready = true;
    return true;
}

void deinitLog() {
    logready = false;
    f_close(&logfile);
    f_unmount("0:");
}

void syncLog() {
    u32 irqState = ARM_EnterCritical();
    f_sync(&logfile);
    ARM_LeaveCritical(irqState);
}

bool logWrite(const void* data, unsigned int btw) {
    if (!logready)
        return false;

    UINT bw;
    u32 irqState = ARM_EnterCritical();
    bool ret = (f_write(&logfile, data, btw, &bw) == FR_OK) && (bw == btw);
    ARM_LeaveCritical(irqState);

    return ret;
}

bool logWriteStr(const char* str) {
    //return logready && logWrite(str, strlen(str));
    return logWrite(str, strlen(str));
}

void logWritevf(const char* format, va_list args) {
    static char strbuf[30];
    u32 i = 0;

    //u32 irqState = ARM_EnterCritical();

    for (;;i++) {
        char c = format[i];

        if (c == '\0') {
            logWrite(format, i);
            break;
        }

        if (c == '%') {
            logWrite(format, i);
            format += i;
            i = 0;
            int lcount = 0;

            codeswitch:
            switch (format[1]) {
                case '%':
                    format++;
                    continue;
                case 's':
                    logWriteStr(va_arg(args, const char*));
                    format += 2;
                    continue;
                case 'd': {
                    s32 arg;
                    if (lcount > 1) {
                        s64 arg64 = va_arg(args, s64);
                        if ((arg64 < (s64) INT32_MIN) || (arg64 > (s64) INT32_MAX))
                            goto default_;
                        else
                            arg = (s32) arg64;
                    } else
                        arg = va_arg(args, s32);
                    itoadec(arg, strbuf);
                    logWriteStr(strbuf);
                    format += 2;
                    continue;
                }
                case 'u': {
                    u32 arg;
                    if (lcount > 1) {
                        u64 arg64 = va_arg(args, u64);
                        if (arg64 > (u64) UINT32_MAX)
                            goto default_;
                        else
                            arg = (u32) arg64;
                    } else
                        arg = va_arg(args, u32);
                    utoadec(arg, strbuf);
                    logWriteStr(strbuf);
                    format += 2;
                    continue;
                }
                case 'x':
                    if (lcount > 1) {
                        u64 arg = va_arg(args, u64);
                        utoahex(arg >> 32, strbuf);
                        if (*strbuf != '0')
                            logWriteStr(strbuf);
                        utoahex((u32) arg, strbuf);
                    } else
                        utoahex(va_arg(args, u32), strbuf);
                    logWriteStr(strbuf);
                    format += 2;
                    continue;
                case 'l':
                    lcount++;
                    format++;
                    goto codeswitch;
                default_:
                default:
                    format -= lcount;
                    continue;
            }
        }
    }

    //ARM_LeaveCritical(irqState);
}

void logWritef(const char* format, ...) {
    va_list args;
    va_start(args, format);

    logWritevf(format, args);

    va_end(args);
}
