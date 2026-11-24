#include "../headers/block_device.h"
#include "../headers/kheap.h"

BlockDevice *block_devices;
u64 block_device_count;
u64 block_device_capacity;

int init_block_device_interface() {
    block_device_capacity = 10; // Initial capacity
    block_device_count = 0;
    block_devices = (BlockDevice *)kalloc(block_device_capacity * sizeof(BlockDevice));

    if (block_devices == NULL) {
        return -1; // Allocation failed
    }

    return 0;
}

int register_block_device(BlockDevice *device) {
    if (block_device_count >= block_device_capacity) {
        // Resize the array
        block_device_capacity *= 2;
        BlockDevice *new_array = (BlockDevice *)kalloc(block_device_capacity * sizeof(BlockDevice));

        if (new_array == NULL) return -1; // Allocation failed

        for (u64 i = 0; i < block_device_count; i++) {
            new_array[i] = block_devices[i];
        }

        block_devices = new_array;
    }

    block_devices[block_device_count++] = *device;

    return 0;
}

int unregister_block_device(u32 device_id) {
    BlockDevice *device = get_block_device(device_id);

    if (device == NULL) return -1; // Device not found

    // do a swap remove manoeuvre

    memcpy(&block_devices[block_device_count-1], device, sizeof(BlockDevice));

    block_device_count--;

    return 0;
}

BlockDevice *get_block_device(u32 device_id) {
    for (u64 i = 0; i < block_device_count; i++) {
        if (block_devices[i].id == device_id) {
            return &block_devices[i];
        }
    }
    return NULL; // Device not found
}