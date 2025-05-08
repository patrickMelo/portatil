OUTPUT_ARCH("riscv")
ENTRY(main)

MEMORY {
    PROGRAM (r): ORIGIN = 0, LENGTH = 64K
}

SECTIONS
{
    .text : ALIGN(8) {
        *(.text*.main);
        *(.text*);
    } > PROGRAM
}