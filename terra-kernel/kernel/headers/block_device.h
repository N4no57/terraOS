#include "../headers/utils.h"

typedef struct BlockDevice {
    u32 id;

    u64 sector_count;
    u32 sector_size;
    
    int (*read_sector)(struct BlockDevice *device, u32 lba, u32 count, void *buffer);

    int (*write_sector)(struct BlockDevice *device, u32 lba, u32 count, void *buffer);

    void *driver_data; // Pointer to driver-specific data
} BlockDevice;

int init_block_device_interface();
int register_block_device(BlockDevice *device);
int unregister_block_device(u32 device_id);
BlockDevice *get_block_device(u32 device_id);