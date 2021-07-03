MEMORY
{
	/* Part has 256k of flash */
    flash (rx) : ORIGIN = 0x00000000, LENGTH = 0x00040000
	/* Ram has two regions (SRAM1 and SRAM2), but they are mapped contiguously.
	   SRAM2 can retain its contents in low power modes */
	ram (rw) : ORIGIN = 0x20000000, LENGTH = 0x00010000
}
  
SECTIONS
{
    /* initial stack pointer pointing to top of stack */
    PROVIDE( _stack_ptr = ORIGIN(ram) + LENGTH(ram));

    /* Start linker at 0x0000 */
	. = ORIGIN(flash);
    .text : 
	{
		_stext = .;
		KEEP(*(.vectors*))
		*(.text*)
		*(.rodata.*)
        _etext = .;
	} > flash 

	.data :
    {
		_srcdata = LOADADDR(.data); /* Where we need to copy .data from in rom */
		_sdata = .;
        *(.data*);
		_edata = .;
    } > ram AT > flash /* .data section is copied from flash to ram at boot */
	
	/* .bss section which is used for uninitialized data */
	.bss (NOLOAD) :
    {
		_sbss = . ;
        *(.bss*)
        *(COMMON)
		_ebss = . ;
    } > ram
}
