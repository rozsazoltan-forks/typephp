BUILD := build
OBJECT_DIR := $(BUILD)/objects
BOOTSTRAP_OBJECT := $(OBJECT_DIR)/bootstrap.o
PAYLOAD_ELF := $(BUILD)/kernel64.elf
PAYLOAD_BIN := $(BUILD)/kernel64.bin
PAYLOAD_OBJECT := $(OBJECT_DIR)/kernel64-payload.o
KERNEL := $(BUILD)/typephp-os.elf

BOOTFLAGS := -m32 -O2 -ffreestanding -fno-pic -fno-pie \
	-fno-stack-protector -Wall -Wextra

.PHONY: all payload objects run test clean

all: $(KERNEL)

payload: objects
	cd ../.. && ./bin/tpc.php --nano examples/typephp-os/project.yml

# Only the 32-bit bootstrap requires flags incompatible with the 64-bit Nano
# payload. Ordinary .c/.cc/.S files stay in project.yml sources.
objects: $(BOOTSTRAP_OBJECT)

$(OBJECT_DIR):
	mkdir -p $@

$(BOOTSTRAP_OBJECT): freestanding/boot.S | $(OBJECT_DIR)
	gcc $(BOOTFLAGS) -c $< -o $@

$(KERNEL): payload
	objcopy -O binary $(PAYLOAD_ELF) $(PAYLOAD_BIN)
	objcopy -I binary -O elf32-i386 -B i386 \
		--rename-section .data=.payload,alloc,load,readonly,data,contents \
		$(PAYLOAD_BIN) $(PAYLOAD_OBJECT)
	ld -m elf_i386 -T freestanding/linker.ld -o $@ \
		$(BOOTSTRAP_OBJECT) $(PAYLOAD_OBJECT)

run: all
	qemu-system-x86_64 -m 128M -kernel $(KERNEL) \
		-display none -serial stdio -monitor none -no-reboot -no-shutdown

test: all
	bash tools/test-qemu.sh $(KERNEL) $(BUILD)/qemu.log

clean:
	rm -rf $(BUILD)
