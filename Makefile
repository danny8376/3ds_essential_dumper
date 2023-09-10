TARGET = 3ds_essential_dumper

ARM9_LOAD      = 0x08000100
ARM9_ENTRY     = $(ARM9_LOAD)
ARM9_ALT_LOAD  = 0x01FF8000
ARM9_ALT_ENTRY = $(ARM9_ALT_LOAD)
ARM9_BIN       = arm9/$(TARGET)_arm9.bin

export CC      = arm-none-eabi-gcc
export OBJCOPY = arm-none-eabi-objcopy

ifeq (, $(shell which $(CC) 2> /dev/null))
    export USE_DKA = true
endif
ifeq (, $(shell which $(OBJCOPY) 2> /dev/null))
    export USE_DKA = true
endif

ifeq ($(USE_DKA),true)
    ifeq ($(strip $(DEVKITARM)),)
        $(error "Please provide $(CC) and $(OBJCOPY) in PATH, or set DEVKITARM in your environment.")
    endif
    $(info Building with DKA)
    include $(DEVKITARM)/base_tools
endif

.PHONY: all clean .FORCE
.FORCE:

all: $(TARGET).firm $(TARGET)_direct.firm

$(TARGET).firm: $(ARM9_BIN)
	firmtool build $@ -n $(ARM9_ENTRY) -D $(ARM9_BIN) -A $(ARM9_LOAD) -C NDMA -b
$(TARGET)_direct.firm: $(ARM9_BIN) $(ARM11_BIN)
	firmtool build $@ -n $(ARM9_ALT_ENTRY) -D $(ARM9_BIN) -A $(ARM9_ALT_LOAD) -C memcpy -b

$(ARM9_BIN): .FORCE
	@$(MAKE) -C arm9

clean:
	@$(MAKE) -C arm9 clean
	@rm -f $(TARGET)*.firm
