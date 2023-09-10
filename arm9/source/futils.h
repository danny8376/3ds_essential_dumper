#pragma once

#include "common.h"
#include "smalllib.h"
#include "fatfs/ff.h"

#include <stdarg.h>

#define MAX_PATH 256

const char* joinStr(char* buf, int bufSize, ...);
const char* rpath(char* buf, const char* file);
FRESULT f_ropen(FIL* fp, char* buf, const TCHAR* path, BYTE mode);
