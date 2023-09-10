#include "futils.h"
#include "fatfs/ff.h"

const char* joinStr(char *buf, int bufSize, ...) { // Add NULL at last for safety
    int len = 0;
    const char *path;
    char *cbuf = buf;

    va_list args;
    va_start(args, bufSize);

    while( ( path = va_arg(args, const char *) ) != NULL ) {
        int plen = strlen(path);
        if (len + plen >= bufSize) {
            break;
        } else {
            memcpy(cbuf, path, plen);
            len += plen;
            cbuf += plen;
        }
    }

    cbuf[0] = 0;

    va_end(args);

    return buf;
}

const char* rpath(char *buf, const char *file) {
    return joinStr(buf, MAX_PATH, MAIN_PATH, file, NULL);
}

FRESULT f_ropen(FIL *fp, char *buf, const TCHAR *path, BYTE mode) {
    return f_open(fp, rpath(buf, path), mode);
}
