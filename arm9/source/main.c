#include "common.h"
#include "hid.h"
#include "log.h"
#include "i2c.h"
#include "timer.h"
#include "irq.h"
#include "irqHandlers.h"
#include "smalllib.h"
#include "fatfs/ff.h"
#include "fatfs/sdmmc.h"

#include <power.h>
#include <pxi.h>
#include <debug.h>
#include <arm.h>

#include <stdlib.h>

//#define TITLE_STRING ("===== 3ds_essential_dumper by DannyAAM - commit " COMMIT " =====\n")
#define TITLE_STRING ("======\n")

#define STR_FAILED "Failed!(%d)\n"
#define STR_DUMPING "Dumping %s...\n"
#define STR_DUMPED "%s dumped\n"
//#define STR_DUMPED_S "%s dumped (%u written)\n"
//#define STR_CANT_READ "Can't read %s from NAND! (%d)\n"
#define STR_CANT_READ "Can't read %s!(%d)\n"
//#define STR_CANT_CREATE "Failed to create %s on SD! (%d)\n"
#define STR_CANT_CREATE "Can't create %s!(%d)\n"
#define STR_DUMP_FAILED "Dump failed!(%d)\n"

#define MAX_PATH 256

#define GBUF_SZ   0x1000
#define GBUF_SECS (GBUF_SZ / 0x200)
u8 __attribute__((aligned(4))) gBuf[GBUF_SZ]; // 4KiB

static u32 wait_any_key_pressed()
{
    u32 pad;
    while (~HID_PAD & BUTTON_NONE);
    while (!((pad = ~HID_PAD) & BUTTON_NONE));
    return pad;
}

const char* joinStr(char* buf, int bufSize, ...) { // Add NULL at last for safety
    int len = 0;
    const char* path;
    char* cbuf = buf;

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

const char* rpath(char* buf, const char* file) {
    return joinStr(buf, MAX_PATH, MAIN_PATH, file, NULL);
}

FRESULT fileCopy(const char *srcDir, const char *fn, bool optional) {
    FIL src, dst;
    FRESULT res;
    char srcPath[MAX_PATH], dstPath[MAX_PATH];

    logWritef(STR_DUMPING, fn);

    joinStr(srcPath, MAX_PATH, srcDir, fn, NULL);
    rpath(dstPath, fn);

    res = f_open(&src, srcPath, FA_READ);
    if (res != FR_OK) {
        if (!optional)
            logWritef(STR_CANT_READ, fn, res);
        return res;
    }

    FSIZE_t rem = f_size(&src);

    res = f_open(&dst, dstPath, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        f_close(&src);
        logWritef(STR_CANT_CREATE, dstPath, res);
        return res;
    }

    while (rem > 0) {
        FSIZE_t sz = rem >= GBUF_SZ ? GBUF_SZ : rem;
        UINT n;

        res = f_read(&src, gBuf, sz, &n);
        if (n != sz)
            res = FR_INT_ERR; // ???

        if (res == FR_OK) {
            res = f_write(&dst, gBuf, sz, &n);
            if (n != sz)
                res = FR_DENIED;
        }

        if (res != FR_OK) {
            f_close(&dst);
            f_close(&src);
            f_unlink(dstPath);
            //logWritef("Failed to copy %s! (%d)\n", fn, res);
            //logWritef(STR_DUMP_FAILED, res);
            logWritef(STR_FAILED, res);
            return res;
        }

        rem -= sz;
    }

    f_close(&dst);
    f_close(&src);

    logWritef(STR_DUMPED, fn);
    return FR_OK;
}

FRESULT nandDump(u32 offset, u32 sectors, const char *fn, const char *dispName, bool ignoreNandFail) {
    FIL dst;
    FRESULT fres;
    int res;
    char dstPath[MAX_PATH];

    logWritef("Dumping %s (size %u)...\n", dispName, sectors * 0x200);

    rpath(dstPath, fn);

    fres = f_open(&dst, dstPath, FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK) {
        logWritef(STR_CANT_READ, dstPath, fres);
        return fres;
    }

    while (sectors > 0) {
        u32 cnt = sectors >= GBUF_SECS ? GBUF_SECS : sectors;
        u32 sz = cnt * 0x200;
        UINT n;

        res = sdmmc_nand_readsectors(offset, cnt, gBuf);
        if (res != 0) {
            logWritef("Can't read NAND %u-%u! (%d)\n", offset, offset + cnt, res);
            if (!ignoreNandFail)
                fres = FR_INT_ERR;
        } else {
            fres = f_write(&dst, gBuf, sz, &n);
            if (n != sz)
                fres = FR_DENIED;
        }

        if (fres != FR_OK) {
            f_close(&dst);
            f_unlink(dstPath);
            //logWritef("Failed to dump %s! (%d)\n", dispName, fres);
            //logWritef(STR_DUMP_FAILED, fres);
            logWritef(STR_FAILED, fres);
            return fres;
        }

        offset += cnt;
        sectors -= cnt;
    }

    f_close(&dst);

    logWritef(STR_DUMPED, dispName);
    return FR_OK;
}

int main(/*int argc, char *argv[]*/) {
    FIL fp;
    FATFS fs;
    UINT bw;
    FRESULT fres;
    int res;
    u32 hid;
    char path[MAX_PATH];

    PXI_Reset();
    resetAllTimers();
    disableAllInterrupts();
    clearAllInterrupts();

    if (!initLog()) { // sd init failed, nothing to do, power off
        // maybe blink code here
        mcuPoweroff();
    }

    logWriteStr(TITLE_STRING);

    logWritef(STR_DUMPING, "OTP");
    fres = f_open(&fp, rpath(path, "otp.bin"), FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK) {
        logWritef(STR_CANT_CREATE, path, fres);
    } else {
        f_write(&fp, (u8*)0x10012000, 0x100, &bw);
        f_close(&fp);
        //logWritef(STR_DUMPED_S, "OTP", bw);
        logWritef(STR_DUMPED, "OTP");
    }

    logWritef(STR_DUMPING, "NAND CID");
    fres = f_open(&fp, rpath(path, "nand_cid.bin"), FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK) {
        logWritef(STR_CANT_CREATE, path, fres);
    } else {
        sdmmc_get_cid(1, (u32 *)gBuf);
        f_write(&fp, gBuf, 0x10, &bw);
        f_close(&fp);
        //logWritef(STR_DUMPED_S, "NAND CID", bw);
        logWritef(STR_DUMPED, "NAND CID");
    }

    nandDump(0, 1, "nand_hdr.bin", "NAND NCSD Header", false);

    res = sdmmc_nand_readsectors(1, 1, gBuf);
    if (res != 0 && memcmp(gBuf, "nand_hdr", 8) == 0)
        nandDump(1, 17, "essential.exefs", "GM9 Essential", false);

    //setupKeyslots();

    logWriteStr("Mounting NAND...\n");
    fres = f_mount(&fs, "1:", 1);
    if (fres != FR_OK) {
        //logWritef("Fail to mount NAND! (%d)\n", fres);
        logWritef(STR_FAILED, fres);
    } else {
        fileCopy("1:/ro/sys/", "HWCAL0.dat", false);
        fileCopy("1:/ro/sys/", "HWCAL1.dat", false);

        fileCopy("1:/rw/sys/", "LocalFriendCodeSeed_A", false);
        fileCopy("1:/rw/sys/", "LocalFriendCodeSeed_B", false);

        fileCopy("1:/rw/sys/", "SecureInfo_A", false);
        fileCopy("1:/rw/sys/", "SecureInfo_B", true);
        fileCopy("1:/rw/sys/", "SecureInfo_C", true); // Probably not needed?

        fileCopy("1:/private/", "movable.sed", false);
    }

    //mcuSetInfoLedPattern(64, 64, 64, 100, false);
    I2C_writeReg(I2C_DEV_CTR_MCU, 0x29, 6);

    hid = wait_any_key_pressed();
    if (!(hid & (BUTTON_B | BUTTON_Y))) {
        //mcuSetInfoLedPattern(0, 0, 0, 0, false);
        I2C_writeReg(I2C_DEV_CTR_MCU, 0x29, 1);

        nandDump(0, getMMCDevice(0)->total_size, "nand.bin", "Full NAND", hid & (BUTTON_SELECT | BUTTON_START));
    }

    deinitLog();

    mcuPoweroff();
    while (1);
}
