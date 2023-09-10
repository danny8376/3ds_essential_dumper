# 3ds_essential_dumper
Tiny program for dumping essential files from 3DS console, which hopefully works on
consoles that have dead FCRAM / NAND and broken screen.

## General Info
This program runs solely on ARM9 processor and only use ITCM/DTCM, thus should hopefully
works on consoles that have dead FCRAM (and maybe even VRAM/ARM9 Memory?)

It'll try to dump all these files whenever possible:
* OTP
* NAND CID
* NAND NCSD Header
* Embed GodMode9 Essential Backup
* If CTRNAND is mountable
  - HWCAL0/1
  - LocalFriendCodeSeed
  - SecureInfo
  - movable.sed
* Full NAND

Before dumping full NAND, it'll blink red power led (the same as low power indicator),
you can press Y/B to cancel it, or any other button to continue dumping.

## To-Do List / Roadmap
There're no space left in ITCM, so we need to somehow shrink things to be able to fit
anything more below :(
* setupKeyslots on init
  - seems to be required if booting directly from ntrboot?
* Proper title in log file

## How to Build This
Building requires [firmtool](https://github.com/TuxSH/firmtool), and either 
`arm-none-eabi-gcc` and `arm-none-eabi-objcopy` in path or DevkitARM installed.
Then, just run `make`.

## License
You may use this under the terms of the GNU General Public License GPL v2 or
under the terms of any later revisions of the GPL. Refer to the provided
`license.txt` file for further information.

## Credits
* **aspargas2**, this whole repo is heavily based on [3ds_hw_test](https://github.com/aspargas2/3ds_hw_test/commit/88d0c4be718fcf1ec69f272ad29301262a0c6b48)
* **LumaTeam**, for crypto.c / diskio.c / sdmmc.c / mcuSetInfoLedPattern code, from (old) boot9strap code
* **The Chromium OS Authors**, for thumb_case.S (__gnu_thumb1_case_uqi) code
* **Jörg Mische**, for uidiv.S (__aeabi_uidiv) code
* **Cha(N)**, **Kane49**, and all other FatFS contributors for FatFS
* All **[3dbrew.org](https://www.3dbrew.org/wiki/Main_Page) editors**
