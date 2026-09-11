OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
    . = USER_BASE + SIZEOF_HEADERS;

    .text : { *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*) }
    .data : { *(.data .data.*) }
    .bss (NOLOAD) : { *(COMMON) *(.bss .bss.*) }

    /DISCARD/ : { *(.comment) *(.eh_frame*) *(.note*) }
}
