OUTPUT_FORMAT(binary)
ENTRY(start)

SECTIONS
{
    . = 0x7C3E;
    .text ALIGN(1) :
    {
        *(.text)
        *(.data)
    } = 0x00

    .sig 0x7DFE :
    {
        *(.magic)
    }

    .extended_bootloader 0x7E00 :
    {
        *(.extended_bootloader)
    }

    . = 0x8000;

    /DISCARD/ :
    {
        *(.bss*)
        *(*)
    }
}