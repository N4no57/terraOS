OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x7C3E;
    .text ALIGN(1) :
    {
        *(.text)
        *(.data)
    } = 0x00

    /DISCARD/ :
    {
        *(.bss*)
        *(*)
    }
}