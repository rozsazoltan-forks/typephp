BUILD := build
OBJECT_DIR := $(BUILD)/objects
BOOTSTRAP_OBJECT := $(OBJECT_DIR)/bootstrap.o
KERNEL64_ENTRY_OBJECT := $(OBJECT_DIR)/kernel64-entry.o
PAYLOAD_ELF := $(BUILD)/kernel64.elf
PAYLOAD_BIN := $(BUILD)/kernel64.bin
PAYLOAD_OBJECT := $(OBJECT_DIR)/kernel64-payload.o
KERNEL := $(BUILD)/typephp-os.elf
DISK := $(BUILD)/typephp-os.img
USER_COMMANDS := ls cd date uname pwd cat echo write touch mkdir rm rmdir mv fault vmfault wrfault memtest
USER_PROGRAMS := sh $(USER_COMMANDS)
USER_SH_ELF := $(BUILD)/sh.elf
USER_COMMAND_ELFS := $(addprefix $(BUILD)/,$(addsuffix .elf,$(USER_COMMANDS)))
USER_CRT0_OBJECT := $(OBJECT_DIR)/user-crt0.o
USER_LIBC_OBJECTS := $(OBJECT_DIR)/user-libc.o $(OBJECT_DIR)/user-syscall.o
NANO_USER_ELF := $(BUILD)/tnhello.elf
TYPEPHP_OS_LIBRARY := $(BUILD)/libtypephp-os.a
TYPEPHP_OS_LIBRARY_OBJECT_DIR := $(BUILD)/libtypephp-os
PHP_NANO_DIR := $(or $(realpath ../../vendor/swoole/php-nano),$(realpath ../../../php-nano))
PHPX_DIR := $(or $(realpath ../../vendor/swoole/phpx),$(realpath ../../../phpx))
OPENLIBM_SOURCES := $(shell sed -n 's|^\(.*\.c\)$$|thirdparty/openlibm/\1|p' tools/openlibm-files.txt)
TYPEPHP_OS_LIBRARY_C_SOURCES := user/runtime/posix.c kernel/core/abi/libc.c $(OPENLIBM_SOURCES)
TYPEPHP_OS_LIBRARY_CXX_SOURCES := user/runtime/host.cc kernel/core/abi/cxx.cpp
TYPEPHP_OS_LIBRARY_ASM_SOURCES := user/runtime/crt0.S user/runtime/syscall.S \
	kernel/core/abi/unimplemented.S
TYPEPHP_OS_LIBRARY_OBJECTS := \
	$(patsubst %.c,$(TYPEPHP_OS_LIBRARY_OBJECT_DIR)/%.o,$(TYPEPHP_OS_LIBRARY_C_SOURCES)) \
	$(patsubst %.cc,$(TYPEPHP_OS_LIBRARY_OBJECT_DIR)/%.o,$(filter %.cc,$(TYPEPHP_OS_LIBRARY_CXX_SOURCES))) \
	$(patsubst %.cpp,$(TYPEPHP_OS_LIBRARY_OBJECT_DIR)/%.o,$(filter %.cpp,$(TYPEPHP_OS_LIBRARY_CXX_SOURCES))) \
	$(patsubst %.S,$(TYPEPHP_OS_LIBRARY_OBJECT_DIR)/%.o,$(TYPEPHP_OS_LIBRARY_ASM_SOURCES))
TYPEPHP_OS_LIBRARY_DEPFILES := $(addsuffix .d,$(TYPEPHP_OS_LIBRARY_OBJECTS))
NANO_USER_SOURCES := user/nano/hello.php user/nano/hello.yml user/runtime/linker.ld \
	$(TYPEPHP_OS_LIBRARY)

BOOTFLAGS := -m32 -O2 -ffreestanding -fno-pic -fno-pie \
	-fno-stack-protector -Wall -Wextra
KERNEL64_BOOTFLAGS := -m64 -O2 -ffreestanding -fno-builtin -fno-pic -fno-pie \
	-fno-stack-protector -mno-red-zone -mcmodel=small -Wall -Wextra
USER_CFLAGS := -m64 -O2 -ffreestanding -fno-builtin -fno-pic -fno-pie \
	-fno-stack-protector -mno-red-zone -Wall -Wextra -Iuser/include -Ikernel/core/abi
USER_LDFLAGS := -m elf_x86_64 -nostdlib -static -T user/linker.ld
TYPEPHP_OS_RUNTIME_DEFINES := -DTYPEPHP_NATIVE=1 -DPHP_NANO=1 -DPHPX_NANO=1 \
	-D_POSIX_C_SOURCE=200809L -DTYPEPHP_NO_MAIN=1 -DPHPX_NO_EXCEPTION=1 \
	-DPHPX_NO_RTTI=1 -DPHP_NANO_NO_LIBC=1 -DHAVE_SYS_UTSNAME_H=1 \
	-D__NO_CTYPE=1 -DZEND_MM_ERROR=0 -DZEND_MM_CUSTOM=0 -DASSEMBLER=1 \
	-D__BSD_VISIBLE=1
TYPEPHP_OS_RUNTIME_INCLUDES := -Ikernel/core/abi \
	-Ithirdparty/openlibm/include -Ithirdparty/openlibm/src \
	-Ithirdparty/openlibm/amd64 -I$(PHP_NANO_DIR) -I$(PHP_NANO_DIR)/Zend \
	-I$(PHP_NANO_DIR)/include -I$(PHP_NANO_DIR)/main -I$(PHP_NANO_DIR)/TSRM \
	-I$(PHP_NANO_DIR)/ext/date/lib -I$(PHP_NANO_DIR)/ext/pcre/pcre2lib \
	-I$(PHPX_DIR)/include -I$(PHPX_DIR)/thirdparty/wren-gc/include
TYPEPHP_OS_RUNTIME_CFLAGS := -m64 -O0 -std=c11 -ffreestanding -fno-builtin \
	-fno-pic -fno-pie -fno-stack-protector -mno-red-zone -mcmodel=small \
	-ffunction-sections -fdata-sections -Wall -Wextra \
	$(TYPEPHP_OS_RUNTIME_DEFINES) $(TYPEPHP_OS_RUNTIME_INCLUDES)
TYPEPHP_OS_RUNTIME_CXXFLAGS := -m64 -O0 -std=c++17 -ffreestanding -fno-builtin \
	-fno-pic -fno-pie -fno-stack-protector -mno-red-zone -mcmodel=small \
	-fno-rtti -fno-threadsafe-statics -ffunction-sections -fdata-sections \
	-Wall -Wextra \
	$(TYPEPHP_OS_RUNTIME_DEFINES) $(TYPEPHP_OS_RUNTIME_INCLUDES)
TYPEPHP_OS_RUNTIME_ASMFLAGS := -m64 -ffreestanding -fno-pic -fno-pie \
	-fno-stack-protector -mno-red-zone -mcmodel=small
ARFLAGS := rcs

.PHONY: all thirdparty payload objects user-runtime nano-user disk run test clean

all: $(KERNEL) $(DISK)

thirdparty:
	./tools/fetch-thirdparty.sh

payload: thirdparty objects
	cd ../.. && ./bin/tpc.php --nano examples/typephp-os/project.yml

# Startup sources are built explicitly by Make. The 64-bit entry object is
# injected into tpc's final link through project.yml; the 32-bit Multiboot
# object participates only in the packaging link.
objects: $(BOOTSTRAP_OBJECT) $(KERNEL64_ENTRY_OBJECT)

user-runtime: $(TYPEPHP_OS_LIBRARY)

nano-user: $(TYPEPHP_OS_LIBRARY) $(NANO_USER_ELF)

disk: $(DISK)

$(DISK): $(USER_SH_ELF) $(USER_COMMAND_ELFS) $(NANO_USER_ELF) | $(BUILD)
	truncate -s 32M $@
	mkfs.fat -F 16 -n TYPEPHPOS $@
	mmd -i $@ ::/BIN
	for program in $(USER_PROGRAMS); do \
		mcopy -o -i $@ $(BUILD)/$$program.elf ::/BIN/$$program.elf; \
	done
	mcopy -o -i $@ $(NANO_USER_ELF) ::/BIN/TNHELLO.ELF

$(BUILD):
	mkdir -p $@

$(OBJECT_DIR):
	mkdir -p $@

$(BOOTSTRAP_OBJECT): boot/boot.S | $(OBJECT_DIR)
	gcc $(BOOTFLAGS) -c $< -o $@

$(KERNEL64_ENTRY_OBJECT): boot/kernel64-entry.S | $(OBJECT_DIR)
	gcc $(KERNEL64_BOOTFLAGS) -c $< -o $@

$(OBJECT_DIR)/%.o: user/%.c kernel/core/abi/typephp_os_syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(USER_CRT0_OBJECT): user/crt0.S kernel/core/abi/typephp_os_syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(OBJECT_DIR)/user-libc.o: user/libc.c kernel/core/abi/typephp_os_syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(OBJECT_DIR)/user-syscall.o: user/syscall.S kernel/core/abi/typephp_os_syscall.h | $(OBJECT_DIR)
	gcc $(USER_CFLAGS) -c $< -o $@

$(OPENLIBM_SOURCES): | thirdparty

$(TYPEPHP_OS_LIBRARY_OBJECT_DIR)/%.o: %.c Makefile
	@mkdir -p $(@D)
	gcc $(TYPEPHP_OS_RUNTIME_CFLAGS) -MMD -MP -MF $@.d -c $< -o $@

$(TYPEPHP_OS_LIBRARY_OBJECT_DIR)/%.o: %.cc Makefile
	@mkdir -p $(@D)
	g++ $(TYPEPHP_OS_RUNTIME_CXXFLAGS) -MMD -MP -MF $@.d -c $< -o $@

$(TYPEPHP_OS_LIBRARY_OBJECT_DIR)/%.o: %.cpp Makefile
	@mkdir -p $(@D)
	g++ $(TYPEPHP_OS_RUNTIME_CXXFLAGS) -MMD -MP -MF $@.d -c $< -o $@

$(TYPEPHP_OS_LIBRARY_OBJECT_DIR)/%.o: %.S Makefile
	@mkdir -p $(@D)
	gcc $(TYPEPHP_OS_RUNTIME_ASMFLAGS) -MMD -MP -MF $@.d -c $< -o $@

$(TYPEPHP_OS_LIBRARY): $(TYPEPHP_OS_LIBRARY_OBJECTS) | $(BUILD)
	$(AR) $(ARFLAGS) $@ $^

$(NANO_USER_ELF): $(NANO_USER_SOURCES) | thirdparty $(BUILD)
	cd ../.. && ./bin/tpc.php --nano examples/typephp-os/user/nano/hello.yml

$(USER_SH_ELF): $(OBJECT_DIR)/sh.o $(USER_CRT0_OBJECT) $(USER_LIBC_OBJECTS) user/linker.ld Makefile | $(BUILD)
	ld $(USER_LDFLAGS) --defsym USER_BASE=0x40000000 -o $@ $(filter %.o,$^)

$(USER_COMMAND_ELFS): $(BUILD)/%.elf: $(OBJECT_DIR)/%.o $(USER_CRT0_OBJECT) $(USER_LIBC_OBJECTS) user/linker.ld Makefile | $(BUILD)
	ld $(USER_LDFLAGS) --defsym USER_BASE=0x41000000 -o $@ $(filter %.o,$^)

$(KERNEL): payload
	objcopy -O binary $(PAYLOAD_ELF) $(PAYLOAD_BIN)
	objcopy -I binary -O elf32-i386 -B i386 \
		--rename-section .data=.payload,alloc,load,readonly,data,contents \
		$(PAYLOAD_BIN) $(PAYLOAD_OBJECT)
	ld -m elf_i386 -T boot/linker.ld -o $@ \
		$(BOOTSTRAP_OBJECT) $(PAYLOAD_OBJECT)

run: all
	qemu-system-x86_64 -m 160M -kernel $(KERNEL) \
		-drive file=$(DISK),format=raw,if=ide,index=0 \
		-display none -serial stdio -monitor none -no-reboot -no-shutdown

test: $(KERNEL) $(USER_SH_ELF) $(USER_COMMAND_ELFS) $(NANO_USER_ELF)
	truncate -s 32M $(DISK)
	mkfs.fat -F 16 -n TYPEPHPOS $(DISK)
	mmd -i $(DISK) ::/BIN
	for program in $(USER_PROGRAMS); do \
		mcopy -o -i $(DISK) $(BUILD)/$$program.elf ::/BIN/$$program.elf; \
	done
	mcopy -o -i $(DISK) $(NANO_USER_ELF) ::/BIN/TNHELLO.ELF
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

-include $(TYPEPHP_OS_LIBRARY_DEPFILES)
