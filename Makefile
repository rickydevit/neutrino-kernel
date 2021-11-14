.DEFAULT_GOAL=$(ELF_TARGET)

ARCH 			:= x86_64
BUILD_OUT 		:= ./build
ISO_OUT 		:= ./iso
ELF_TARGET 		:= neutrino.sys
ISO_TARGET 		:= neutrino.iso
DIRECTORY_GUARD  = mkdir -p $(@D)
LD_SCRIPT 		:= $(ARCH).ld

END_PATH 		:= kernel/common kernel/${ARCH} libs/

# compiler and linker
CC 				:= $(ARCH)-elf-gcc

LD 				:= $(ARCH)-elf-ld

# flags
DEFINEFLAGS  	:= -D__$(ARCH:_=-)

INCLUDEFLAGS 	:= -I. \
					-I./kernel/common \
					-I./kernel/$(ARCH) \
					-I./thirdparty/ \
        			-I./libs/libc \
       				-I./libs/ 
					
CFLAGS 			:= 	-g -Wall -Wl,-Wunknown-pragmas -ffreestanding -fpie -fno-stack-protector \
					-mno-red-zone -mno-3dnow -MMD -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
					-O2 -pipe $(INCLUDEFLAGS) $(DEFINEFLAGS)

LDFLAGS 		:= 	-T $(LD_SCRIPT) -nostdlib -zmax-page-size=0x1000 -static -pie \
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

OBJ 			:= $(shell find $(BUILD_OUT) -type f -name '*.o')

# qemu settings
QEMU 			= /mnt/d/Programmi/qemu/qemu-system-${ARCH}.exe
HARD_FLAGS 		= -m 4G -vga std -cpu core2duo
RUN_FLAGS 		= ${HARD_FLAGS} -serial stdio
DEBUG_FLAGS		= ${HARD_FLAGS} -serial file:serial.log -monitor stdio -d cpu_reset -D qemu.log

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
	@echo "[KERNEL $(ARCH)] (ld) $^"
	@${LD} $^ $(LDFLAGS) -o $@

run: $(ISO_TARGET)
	@rm $(BUILD_OUT)/$(ELF_TARGET)
	@${QEMU} -cdrom $< ${RUN_FLAGS}

debug: $(ISO_TARGET)
	@rm $(BUILD_OUT)/$(ELF_TARGET)
	@${QEMU} -cdrom	$< ${DEBUG_FLAGS} 

clear:
	@echo "[KERNEL $(ARCH)] Removing files..."
	@rm -f $(COBJ) $(ASMOBJ) $(SOBJ) \
	$(HEADDEPS) $(BUILD_OUT)/$(ELF_TARGET) $(ISO_TARGET)

cd:	$(ISO_TARGET)
	@echo "[KERNEL $(ARCH)] Preparing iso..."

.PHONY:$(ISO_TARGET)
$(ISO_TARGET): $(ELF_TARGET)
	@mkdir -p iso
	@cp -v $(BUILD_OUT)/$< utils/limine.cfg limine/limine.sys \
    	limine/limine-cd.bin limine/limine-eltorito-efi.bin $(ISO_OUT)
	@xorriso -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-eltorito-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        $(ISO_OUT) -o $@
	@./limine/limine-install $@