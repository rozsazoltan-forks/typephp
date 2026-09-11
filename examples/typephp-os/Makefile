BUILD := build
OBJECT_DIR := $(BUILD)/objects
BOOTSTRAP_OBJECT := $(OBJECT_DIR)/bootstrap.o
PAYLOAD_ELF := $(BUILD)/kernel64.elf
PAYLOAD_BIN := $(BUILD)/kernel64.bin
PAYLOAD_OBJECT := $(OBJECT_DIR)/kernel64-payload.o
KERNEL := $(BUILD)/typephp-os.elf
DISK := $(BUILD)/typephp-os.img
USER_COMMANDS := ls cd date pwd cat echo write touch mkdir rm rmdir mv fault vmfault wrfault memtest
USER_PROGRAMS := sh $(USER_COMMANDS)
USER_SH_ELF := $(BUILD)/sh.elf
USER_COMMAND_ELFS := $(addprefix $(BUILD)/,$(addsuffix .elf,$(USER_COMMANDS)))
USER_CRT0_OBJECT := $(OBJECT_DIR)/user-crt0.o
USER_LIBC_OBJECTS := $(OBJECT_DIR)/user-libc.o $(OBJECT_DIR)/user-syscall.o

BOOTFLAGS := -m32 -O2 -ffreestanding -fno-pic -fno-pie \
	-fno-stack-protector -Wall -Wextra
USER_CFLAGS := -m64 -O2 -ffreestanding -fno-builtin -fno-pic -fno-pie \
	-fno-stack-protector -mno-red-zone -Wall -Wextra -Iuser/include -Ifreestanding/abi
USER_LDFLAGS := -m elf_x86_64 -nostdlib -static -T user/linker.ld

.PHONY: all thirdparty payload objects disk run test clean

all: $(KERNEL) $(DISK)

thirdparty:
	./tools/fetch-thirdparty.sh

payload: thirdparty objects
	cd ../.. && ./bin/tpc.php --nano examples/typephp-os/project.yml

# Only the 32-bit bootstrap requires flags incompatible with the 64-bit Nano
# payload. Ordinary .c/.cc/.S files stay in project.yml sources.
objects: $(BOOTSTRAP_OBJECT)

disk: $(DISK)

$(DISK): $(USER_SH_ELF) $(USER_COMMAND_ELFS) | $(BUILD)
	truncate -s 32M $@
	mkfs.fat -F 16 -n TYPEPHPOS $@
	mmd -i $@ ::/BIN
	for program in $(USER_PROGRAMS); do \
		mcopy -o -i $@ $(BUILD)/$$program.elf ::/BIN/$$program.elf; \
	done

$(BUILD):
	mkdir -p $@

$(OBJECT_DIR):
	mkdir -p $@

$(BOOTSTRAP_OBJECT): freestanding/boot.S | $(OBJECT_DIR)
	gcc $(BOOTFLAGS) -c $< -o $@

$(OBJECT_DIR)/%.o: user/%.c freestanding/abi/typephp_os_syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(USER_CRT0_OBJECT): user/crt0.S freestanding/abi/typephp_os_syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(OBJECT_DIR)/user-libc.o: user/libc.c freestanding/abi/typephp_os_syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(OBJECT_DIR)/user-syscall.o: user/syscall.S freestanding/abi/typephp_os_syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(USER_SH_ELF): $(OBJECT_DIR)/sh.o $(USER_CRT0_OBJECT) $(USER_LIBC_OBJECTS) user/linker.ld | $(BUILD)
	ld $(USER_LDFLAGS) --defsym USER_BASE=0x2000000 -o $@ $(filter %.o,$^)

$(USER_COMMAND_ELFS): $(BUILD)/%.elf: $(OBJECT_DIR)/%.o $(USER_CRT0_OBJECT) $(USER_LIBC_OBJECTS) user/linker.ld | $(BUILD)
	ld $(USER_LDFLAGS) --defsym USER_BASE=0x2100000 -o $@ $(filter %.o,$^)

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

test: $(KERNEL) $(USER_SH_ELF) $(USER_COMMAND_ELFS)
	truncate -s 32M $(DISK)
	mkfs.fat -F 16 -n TYPEPHPOS $(DISK)
	mmd -i $(DISK) ::/BIN
	for program in $(USER_PROGRAMS); do \
		mcopy -o -i $(DISK) $(BUILD)/$$program.elf ::/BIN/$$program.elf; \
	done
	# An alias not known while linking the kernel verifies disk-based discovery.
	mcopy -o -i $(DISK) $(BUILD)/echo.elf ::/BIN/hello.elf
	mcopy -o -i $(DISK) tools/invalid-elf.txt ::/BIN/bad.elf
	# Fill one directory cluster (dot entries plus 62 files). The QEMU test
	# creates F62.TXT and therefore exercises directory-chain growth once.
	mmd -i $(DISK) ::/EXPAND
	for index in $$(seq -w 0 61); do \
		mcopy -o -i $(DISK) tools/invalid-elf.txt ::/EXPAND/F$$index.TXT; \
	done
	bash tools/test-qemu.sh $(KERNEL) $(DISK) $(BUILD)/qemu.log

clean:
	rm -rf $(BUILD)
