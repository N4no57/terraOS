OUTPUT_FORMAT("elf64-x86-64")
ENTRY(kernel_main)

SECTIONS
{
    . = 0xFFFFFFFF80010000;

    .text ALIGN(0x10) :
    {
		*(.kernel)
        *(.text)
    }

	.rodata ALIGN(0x10) :
	{
		*(.rodata)
	}

	.data ALIGN(0x10) :
	{
		*(.data)
	}

	.bss ALIGN(0x10) :
	{
		*(.bss)
	}
}