#include "../headers/utils.h"
#include "../headers/fdc.h"

#define FLOPPY_144_SECTORS_PER_TRACK 18 // TODO do NOT trust this long term; placeholder until we can read it from the drive...

typedef enum {
    STATUS_REGISTER_A                = 0x3F0, // read-only
    STATUS_REGISTER_B                = 0x3F1, // read-only
    DIGITAL_OUTPUT_REGISTER          = 0x3F2,
    TAPE_DRIVE_REGISTER              = 0x3F3,
    MAIN_STATUS_REGISTER             = 0x3F4, // read-only
    DATARATE_SELECT_REGISTER         = 0x3F4, // write-only
    DATA_FIFO                        = 0x3F5,
    DIGITAL_INPUT_REGISTER           = 0x3F7, // read-only
    CONFIGURATION_CONTROL_REGISTER   = 0x3F7  // write-only
} FloppyRegisters;

typedef enum {
	DOR_MASK_DRIVE0			=	0,	//00000000	= here for completeness sake
	DOR_MASK_DRIVE1			=	1,	//00000001
	DOR_MASK_DRIVE2			=	2,	//00000010
	DOR_MASK_DRIVE3			=	3,	//00000011
	DOR_MASK_RESET			=	4,	//00000100
	DOR_MASK_DMA			=	8,	//00001000
	DOR_MASK_DRIVE0_MOTOR		=	16,	//00010000
	DOR_MASK_DRIVE1_MOTOR		=	32,	//00100000
	DOR_MASK_DRIVE2_MOTOR		=	64,	//01000000
	DOR_MASK_DRIVE3_MOTOR		=	128	//10000000
} DOR_MASK;

typedef enum {
	MSR_MASK_DRIVE1_POS_MODE	=	1,	//00000001
	MSR_MASK_DRIVE2_POS_MODE	=	2,	//00000010
	MSR_MASK_DRIVE3_POS_MODE	=	4,	//00000100
	MSR_MASK_DRIVE4_POS_MODE	=	8,	//00001000
	MSR_MASK_BUSY			    =	16,	//00010000
	MSR_MASK_DMA			    =	32,	//00100000
	MSR_MASK_DATAIO			    =	64, //01000000
	MSR_MASK_DATAREG		    =	128	//10000000
} MSR_MASK;

typedef enum {
   READ_TRACK =                 2,	    // generates IRQ6
   SPECIFY =                    3,      // * set drive parameters
   SENSE_DRIVE_STATUS =         4,
   WRITE_DATA =                 5,      // * write to the disk
   READ_DATA =                  6,      // * read from the disk
   RECALIBRATE =                7,      // * seek to cylinder 0
   SENSE_INTERRUPT =            8,      // * ack IRQ6, get status of last command
   WRITE_DELETED_DATA =         9,
   READ_ID =                    10,	    // generates IRQ6
   READ_DELETED_DATA =          12,
   FORMAT_TRACK =               13,     // *
   DUMPREG =                    14,
   SEEK =                       15,     // * seek both heads to cylinder X
   VERSION =                    16,	    // * used during initialization, once
   SCAN_EQUAL =                 17,
   PERPENDICULAR_MODE =         18,	    // * used during initialization, once, maybe
   CONFIGURE =                  19,     // * set controller parameters
   LOCK =                       20,     // * protect controller params from a reset
   VERIFY =                     22,
   SCAN_LOW_OR_EQUAL =          25,
   SCAN_HIGH_OR_EQUAL =         29
} FloppyCommands;

void lba_2_chs(u32 lba, u16 *cyl, u16 *head, u16 *sector) {
    *cyl    = lba / (2 * FLOPPY_144_SECTORS_PER_TRACK);
    *head   = ((lba % (2 * FLOPPY_144_SECTORS_PER_TRACK)) / FLOPPY_144_SECTORS_PER_TRACK);
    *sector = ((lba % (2 * FLOPPY_144_SECTORS_PER_TRACK)) % FLOPPY_144_SECTORS_PER_TRACK + 1);
}

u8 fdc_check_status() {
    return inb(MAIN_STATUS_REGISTER);
}

// TODO implement ISA DMA for fdc

void fdc_send_command(FloppyCommands command) {
    for (int i = 0; i < 500; i++) {
        if (fdc_check_status() & MSR_MASK_DATAREG) { // check if the controller is ready to accept a command
            outb(DATA_FIFO, command);
            return;
        }
    }
}

u8 fdc_read_data() {
    for (int i = 0; i < 500; i++) {
        if (fdc_check_status() & MSR_MASK_DATAREG) {
            return inb(DATA_FIFO);
        }
    }
    return 0; // return smth if no data
}

int fdc_init() {
    fdc_send_command(VERSION);

    if (fdc_read_data() != 0x90) {
        // Handle error: FDC not present or not responding correctly
        return -1;
    }

    // TODO configure fdc parameters

    if (fdc_reset() != 0) {
        return -1;
    }

    // TODO send recalibrate command to each drive

    return 0;
}

int fdc_reset() {
    

    return 0;
}

void read_sector(u32 lba, u8 *buffer) {
    u16 cyl, head, sector;
    lba_2_chs(lba, &cyl, &head, &sector);

    // TODO implement this function
}

void write_sector(u32 lba, u8 *buffer) {
    u16 cyl, head, sector;
    lba_2_chs(lba, &cyl, &head, &sector);

    // TODO implement this function

}