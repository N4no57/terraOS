OUTPUT_FORMAT(binary)
ENTRY(start)

SECTIONS
{
    . = 0x7C00;

    .text :
    {
        *(.text)
        *(.rodata*)
        *(.data*)
    } = 0x00

    .sig 0x7DFE :
    {
        *(.magic)
    }

    /DISCARD/ :
    {
        *(.bss*)
        *(*)
    }
}