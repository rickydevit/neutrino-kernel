.DEFAULT_GOAL=$(ELF_TARGET)

ARCH 			:= x86_64
BOOTLOADER		:= limine
BUILD_OUT 		:= ./build
ISO_OUT 		:= ./iso
ELF_TARGET 		:= neutrino.sys
ISO_TARGET 		:= neutrino.iso
INITRD_SCRIPT	:= make-initrd
INITRD_TARGET	:= initrd.img
DIRECTORY_GUARD  = mkdir -p $(@D)
LD_SCRIPT 		:= $(ARCH).ld

END_PATH 		:= libs/ kernel/common kernel/${ARCH} 

# compiler and linker
CC 				:= $(ARCH)-elf-gcc

LD 				:= $(ARCH)-elf-ld

# flags
DEFINEFLAGS  	:= -D__$(ARCH) -D__$(BOOTLOADER) -D__kernel

INCLUDEFLAGS 	:= -I. \
					-I./kernel/common \
					-I./kernel/$(ARCH) \
        			-I./libs/libc \
       				-I./libs/ 
					
CFLAGS 			:= 	-g -Wall -Wl,-Wunknown-pragmas -ffreestanding -fpie -fno-stack-protector \
					-mno-red-zone -mno-3dnow -MMD -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
					-O2 -pipe $(INCLUDEFLAGS) $(DEFINEFLAGS)

LDFLAGS 		:= 	-T $(LD_SCRIPT) -nostdlib -zmax-page-size=0x1000 -static \
					--no-dynamic-linker -ztext

# sources and objects
CFILES 			:= $(shell find $(END_PATH) -type f -name '*.c')
CHEADS 			:= $(shell find $(END_PATH) -type f -name '*.h')
COBJ 			:= $(patsubst %.c,$(BUILD_OUT)/%.o,$(CFILES))
HEADDEPS 		:= $(CFILES:.c=.d)

ASMFILES		:= $(shell find $(END_PATH) -type f -name '*.asm') 
SFILES 			:= $(shell find $(END_PATH) -type f -name '*.s')
ASMOBJ			:= $(patsubst %.asm,$(BUILD_OUT)/%.o,$(ASMFILES)) 
SOBJ			:= $(patsubst %.s,$(BUILD_OUT)/%.o,$(SFILES))

INITRDFILES		:= $(shell find initrd/ -type f -name '*')

OBJ 			:= $(shell find $(BUILD_OUT) -type f -name '*.o')

# qemu settings
QEMU 			= qemu-system-${ARCH}
HARD_FLAGS 		= -m 4G -vga std -cpu Skylake-Client -smp 2
RUN_FLAGS 		= ${HARD_FLAGS} -serial stdio -d cpu_reset,int -D qemu.log
DEBUG_FLAGS		= ${HARD_FLAGS} -serial file:serial.log -s -S

# gdb settings
GDB				= gdb
GDB_FLAGS 		= -ex "target remote localhost:1234" -ex "layout split" -ex "set scheduler-locking step"

# === COMMANDS AND BUILD ========================

-include $(HEADDEPS)
$(BUILD_OUT)/%.o: %.c
	@$(DIRECTORY_GUARD)
	@echo "[KERNEL $(ARCH)] (c) $<"
	@${CC} ${CFLAGS} -c $< -o $@

$(BUILD_OUT)/%.o: %.cpp
	@$(DIRECTORY_GUARD)
	@echo "[KERNEL $(ARCH)] (c++) $<"
	@${CCPP} ${CFLAGS} -c $< -o $@

$(BUILD_OUT)/%.o: %.s
	@$(DIRECTORY_GUARD)
	@echo "[KERNEL $(ARCH)] (asm) $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_OUT)/%.o: %.asm
	@$(DIRECTORY_GUARD)
	@echo "[KERNEL $(ARCH)] (asm) $<"
	@nasm $< -f elf64 -o $@

$(ELF_TARGET): $(BUILD_OUT)/$(ELF_TARGET)

.PHONY:$(BUILD_OUT)/$(ELF_TARGET)
$(BUILD_OUT)/$(ELF_TARGET): $(COBJ) $(ASMOBJ)
	@echo "[NEUTRINO] Build started. Current target platform is \"$(ARCH)\", bootloader is \"$(BOOTLOADER)\"."
	@sleep 2s
	@echo "[KERNEL $(ARCH)] (ld) $^"
	@${LD} $^ $(LDFLAGS) -o $@

run: $(ISO_TARGET)
	@${QEMU} -cdrom $< ${RUN_FLAGS}

debug: $(ISO_TARGET)
	@${QEMU} -cdrom	$< ${DEBUG_FLAGS} &
	@${GDB} ${BUILD_OUT}/${ELF_TARGET} ${GDB_FLAGS}

clear:
	@echo "[KERNEL $(ARCH)] Removing files..."
	@rm -f $(COBJ) $(ASMOBJ) $(SOBJ) \
	$(HEADDEPS) $(BUILD_OUT)/$(ELF_TARGET) $(ISO_TARGET)

cd:	$(ISO_TARGET)
	@echo "[KERNEL $(ARCH)] Preparing iso..."

.PHONY:$(ISO_TARGET)
$(ISO_TARGET): $(ELF_TARGET) $(INITRD_TARGET)
	@mkdir -p iso
	@cp -v $(BUILD_OUT)/$< utils/limine.cfg limine/limine.sys \
    	limine/limine-cd.bin limine/limine-eltorito-efi.bin $(INITRD_TARGET) $(ISO_OUT)
	@xorriso -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-eltorito-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        $(ISO_OUT) -o $@
	@./limine/limine-install $@

.PHONY:$(INITRD_SCRIPT)
$(INITRD_SCRIPT):
	@echo "[INITRD] Building \"./make-initrd/\" script..."
	@gcc utils/$@.c -o $@

.PHONY:$(INITRD_TARGET)
$(INITRD_TARGET): $(INITRD_SCRIPT)
	@mkdir -p initrd
	@echo "[INITRD] Adding files from ./initrd/ to $@..."
	@./$(INITRD_SCRIPT) $(INITRDFILES)

initrd: $(INITRD_TARGET)
