.DEFAULT_GOAL=$(ELF_TARGET)

ARCH := x86_64
DIRECTORY_GUARD = mkdir -p $(@D)
BUILD_OUT := ./build
ISO_OUT := ./iso
ELF_TARGET := neutrino.sys
ISO_TARGET := neutrino.iso

END_PATH := kernel/common kernel/${ARCH} libs/libc

CFILES 		:= $(shell find $(END_PATH) -type f -name '*.c')
CHEADS 		:= $(shell find $(END_PATH) -type f -name '*.h')
COBJ 		:= $(patsubst %.c,$(BUILD_OUT)/%.o,$(CFILES))
HEADDEPS 	:= $(CFILES:.c=.d)

ASMFILES	:= $(shell find $(END_PATH) -type f -name '*.asm')
AMSOBJ		:= $(patsubst %.asm,$(BUILD_OUT)/%.o,$(ASMFILES))
				
OBJ = $(shell find $(BUILD_OUT) -type f -name '*.o')

DEFINEFLAGS  := -D__$(ARCH:_=-)

INCLUDEFLAGS := -I./kernel/common \
				-I./kernel/$(ARCH) \
				-I./thirdparty/ \
        		-I./libs/libc \
       			-I./libs/ 

CC := x86_64-elf-gcc
CFLAGS := 	-g -Wall -Wl,--oformat=binary -nostdlib -Wunknown-pragmas -ffreestanding -fpie -fstack-protector \
			-mcmodel=large -mno-red-zone -mno-3dnow -MMD -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
			-fpie -O2 -pipe $(INCLUDEFLAGS) $(DEFINEFLAGS)
LD := x86_64-elf-ld

LD_SCRIPT := $(ARCH).ld
QEMU = /mnt/d/Programmi/qemu/qemu-system-${ARCH}.exe

EMU_MEMORY = 512M
RUN_FLAGS = -m ${EMU_MEMORY} -vga std
DEBUG_FLAGS = ${RUN_FLAGS} -serial file:serial.log -monitor stdio -d guest_errors

# === COMMANDS AND BUILD ========================

-include $(HEADDEPS)
$(BUILD_OUT)/%.o: %.c
	@$(DIRECTORY_GUARD)
	@echo "[KERNEL $(ARCH)] (c) $<"
	${CC} ${CFLAGS} -c $< -o $@

$(BUILD_OUT)/%.o: %.asm
	@$(DIRECTORY_GUARD)
	@echo "[KERNEL $(ARCH)] (asm) $<"
	nasm $< -f elf -o $@

$(ELF_TARGET): $(BUILD_OUT)/$(ELF_TARGET)

.PHONY:$(BUILD_OUT)/$(ELF_TARGET)
$(BUILD_OUT)/$(ELF_TARGET): $(COBJ) $(AMSOBJ)
	${LD} $^ -T$(LD_SCRIPT) -o $@

run: $(ISO_TARGET)
	rm $(BUILD_OUT)/$(ELF_TARGET)
	${QEMU} -cdrom $< ${RUN_FLAGS}

runk: $(BUILD_OUT)/$(ELF_TARGET) 
	${QEMU} -kernel $< ${RUN_FLAGS}

debug: $(ISO_TARGET)
	rm $(BUILD_OUT)/$(ELF_TARGET)
	${QEMU} -cdrom	$< ${DEBUG_FLAGS} 

debugk: $(BUILD_OUT)/$(ELF_TARGET)
	${QEMU} -kernel $< ${DEBUG_FLAGS} 

clear:
	rm -f $(COBJ) $(AMSOBJ) $(HEADDEPS) $(BUILD_OUT)/$(ELF_TARGET) $(ISO_TARGET)

.PHONY:$(ISO_TARGET)
$(ISO_TARGET): $(ELF_TARGET)
	@cp -v $(BUILD_OUT)/$< utils/limine.cfg limine/limine.sys \
    	limine/limine-cd.bin limine/limine-eltorito-efi.bin $(ISO_OUT)
	@xorriso -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-eltorito-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        $(ISO_OUT) -o $@
	@./limine/limine-install $@