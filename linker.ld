OUTPUT_FORMAT(binary)
ENTRY(start)

SECTIONS
{
    . = 0x7C3E;
    .text ALIGN(1) :
    {
        *(.text)
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