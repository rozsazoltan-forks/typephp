BUILD := build
OBJECT_DIR := $(BUILD)/objects
BOOTSTRAP_OBJECT := $(OBJECT_DIR)/bootstrap.o
PAYLOAD_ELF := $(BUILD)/kernel64.elf
PAYLOAD_BIN := $(BUILD)/kernel64.bin
PAYLOAD_OBJECT := $(OBJECT_DIR)/kernel64-payload.o
KERNEL := $(BUILD)/typephp-os.elf
DISK := $(BUILD)/typephp-os.img
USER_SH_ELF := $(BUILD)/sh.elf
USER_LS_ELF := $(BUILD)/ls.elf
USER_CD_ELF := $(BUILD)/cd.elf
USER_IMAGES := $(OBJECT_DIR)/sh-image.o $(OBJECT_DIR)/ls-image.o $(OBJECT_DIR)/cd-image.o

BOOTFLAGS := -m32 -O2 -ffreestanding -fno-pic -fno-pie \
	-fno-stack-protector -Wall -Wextra
USER_CFLAGS := -m64 -O2 -ffreestanding -fno-builtin -fno-pic -fno-pie \
	-fno-stack-protector -mno-red-zone -Wall -Wextra -Iuser
USER_LDFLAGS := -m elf_x86_64 -nostdlib -static -T user/linker.ld

.PHONY: all payload objects disk run test clean

all: $(KERNEL) $(DISK)

payload: objects
	cd ../.. && ./bin/tpc.php --nano examples/typephp-os/project.yml

# Only the 32-bit bootstrap requires flags incompatible with the 64-bit Nano
# payload. Ordinary .c/.cc/.S files stay in project.yml sources.
objects: $(BOOTSTRAP_OBJECT) $(USER_IMAGES)

disk: $(DISK)

$(DISK): | $(BUILD)
	truncate -s 32M $@
	mkfs.fat -F 16 -n TYPEPHPOS $@

$(BUILD):
	mkdir -p $@

$(OBJECT_DIR):
	mkdir -p $@

$(BOOTSTRAP_OBJECT): freestanding/boot.S | $(OBJECT_DIR)
	gcc $(BOOTFLAGS) -c $< -o $@

$(OBJECT_DIR)/sh.o: user/sh.c user/syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(OBJECT_DIR)/ls.o: user/ls.c user/syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(OBJECT_DIR)/cd.o: user/cd.c user/syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(USER_SH_ELF): $(OBJECT_DIR)/sh.o user/linker.ld | $(BUILD)
	ld $(USER_LDFLAGS) --defsym USER_BASE=0x2000000 -o $@ $<

$(USER_LS_ELF): $(OBJECT_DIR)/ls.o user/linker.ld | $(BUILD)
	ld $(USER_LDFLAGS) --defsym USER_BASE=0x2100000 -o $@ $<

$(USER_CD_ELF): $(OBJECT_DIR)/cd.o user/linker.ld | $(BUILD)
	ld $(USER_LDFLAGS) --defsym USER_BASE=0x2200000 -o $@ $<

$(OBJECT_DIR)/sh-image.o: $(USER_SH_ELF) | $(OBJECT_DIR)
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 \
		--rename-section .data=.rodata.user_elf,alloc,load,readonly,data,contents \
		--redefine-sym _binary_build_sh_elf_start=typephp_user_sh_elf_start \
		--redefine-sym _binary_build_sh_elf_end=typephp_user_sh_elf_end $< $@

$(OBJECT_DIR)/ls-image.o: $(USER_LS_ELF) | $(OBJECT_DIR)
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 \
		--rename-section .data=.rodata.user_elf,alloc,load,readonly,data,contents \
		--redefine-sym _binary_build_ls_elf_start=typephp_user_ls_elf_start \
		--redefine-sym _binary_build_ls_elf_end=typephp_user_ls_elf_end $< $@

$(OBJECT_DIR)/cd-image.o: $(USER_CD_ELF) | $(OBJECT_DIR)
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 \
		--rename-section .data=.rodata.user_elf,alloc,load,readonly,data,contents \
		--redefine-sym _binary_build_cd_elf_start=typephp_user_cd_elf_start \
		--redefine-sym _binary_build_cd_elf_end=typephp_user_cd_elf_end $< $@

$(KERNEL): payload
	objcopy -O binary $(PAYLOAD_ELF) $(PAYLOAD_BIN)
	objcopy -I binary -O elf32-i386 -B i386 \
		--rename-section .data=.payload,alloc,load,readonly,data,contents \
		$(PAYLOAD_BIN) $(PAYLOAD_OBJECT)
	ld -m elf_i386 -T freestanding/linker.ld -o $@ \
		$(BOOTSTRAP_OBJECT) $(PAYLOAD_OBJECT)

run: all
	qemu-system-x86_64 -m 128M -kernel $(KERNEL) \
		-drive file=$(DISK),format=raw,if=ide,index=0 \
		-display none -serial stdio -monitor none -no-reboot -no-shutdown

test: all
	bash tools/test-qemu.sh $(KERNEL) $(DISK) $(BUILD)/qemu.log

clean:
	rm -rf $(BUILD)
