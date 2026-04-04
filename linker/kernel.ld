OUTPUT_FORMAT(binary)
ENTRY(kernel_main)

SECTIONS
{
    . = 0xFFFFFFFF80010000;

	.begin :
	{
		*(.kernel)
	}

    .text :
    {
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