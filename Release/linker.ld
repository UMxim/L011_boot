MEMORY{
	ROM(rx) : ORIGIN = 0x08000000, LENGTH = 2K
	SRAM (rwx):     ORIGIN = 0x20000000, LENGTH = 2K
}

_estack = LENGTH(SRAM) + ORIGIN(SRAM);

SECTIONS{
	.isr_vector : {
	KEEP(*(.isr_vector))	
	} >ROM
	
	.text : {
        . = ALIGN(4);
        *(.text)		
    } >ROM	
}