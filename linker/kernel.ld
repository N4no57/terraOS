OUTPUT_FORMAT(binary)
ENTRY(kernel_main)

SECTIONS
{
    . = 0xFFFFFFFF80010000;

    .text :
    {
		*(.kernel)
        *(.text)
    }

	.rodata :
	{
		*(.rodata)
	}

	.data :
	{
		*(.data)
	}

	.bss :
	{
		*(.bss)
	}
}