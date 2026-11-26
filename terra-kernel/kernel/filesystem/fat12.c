#include "../headers/utils.h"
#include "../headers/block_device.h"

#define MAX_PATH 256

typedef struct {
    u16 bytes_per_sector; // Bytes per sector (usually 512)
    u8 sectors_per_cluster; // Sectors per cluster
    u16 reserved_sectors; // Reserved sectors before FAT
    u8 num_fats; // Number of FAT tables (usually 2)
    u16 root_entry_count; // Number of root directory entries
    u8 media_descriptor; // Media descriptor type
    u16 sectors_per_fat; // Sectors per FAT table
    u16 sectors_per_track; // Sectors per track (for CHS addressing)
    u16 num_heads; // Number of heads (for CHS addressing)
    u32 hidden_sectors; // Hidden sectors before the partition
    u32 sector_count; // Total number of sectors
} fat12_header_t;

typedef struct __attribute__((packed)) {
    u8 order; // Order of this entry in the sequence of long name entries
    u16 name1[5]; // First 5 characters of the long name (UTF-16)
    u8 attributes; // Attributes (should be 0x0F for long name entries)
    u8 type; // Type (should be 0 for long name entries)
    u8 checksum; // Checksum of the short name
    u16 name2[6]; // Next 6 characters of the long name (UTF-16)
    u16 first_cluster_low; // Always 0 for long name entries
    u16 name3[2]; // Last 2 characters of the long name (UTF-16)
} fat12_long_name_entry_t;

typedef struct __attribute__((packed)) {
    u8 filename[8]; // Filename (8 characters)
    u8 extension[3]; // File extension (3 characters)
    u8 attributes; // File attributes
    u8 reserved; // Reserved for future use
    u16 creation_time; // Creation time
    u16 creation_date; // Creation date
    u16 last_access_date; // Last access date
    u16 first_cluster_high; // High word of the first cluster number (FAT32 only)
    u16 last_modification_time; // Last modification time
    u16 last_modification_date; // Last modification date
    u16 first_cluster_low; // Low word of the first cluster number
    u32 file_size; // File size in bytes
} fat12_directory_entry_t;

typedef struct __attribute__((packed)) {
    fat12_long_name_entry_t long_name;
    fat12_directory_entry_t entry;
} fat12_directory_t;

typedef struct {
    fat12_header_t metadata;

    u32 cwd;
    char path[MAX_PATH];

    BlockDevice *device;
} FATVolume;

typedef enum fat12_entry_flags {
    FAT_READ_ONLY   = 0x01,
    FAT_HIDDEN      = 0x02,
    FAT_SYSTEM      = 0X04,
    FAT_VOLUME_ID   = 0X08,
    FAT_DIRECTORY   = 0X10,
    FAT_ARCHIVE     = 0X20,
} fat12_entry_flags;

int fat12_mount(FATVolume *volume, BlockDevice *device) {
    u8 boot_sector[512];

    if (device->read_sector(device, 0, 1, boot_sector) != 0) {
        return -1; // Error reading boot sector
    }

    volume->metadata = (fat12_header_t){
        .bytes_per_sector = *(u16 *)(boot_sector + 11),
        .sectors_per_cluster = *(u8 *)(boot_sector + 13),
        .reserved_sectors = *(u16 *)(boot_sector + 14),
        .num_fats = *(u8 *)(boot_sector + 16),
        .root_entry_count = *(u16 *)(boot_sector + 17),
        .media_descriptor = *(u8 *)(boot_sector + 21),
        .sectors_per_fat = *(u16 *)(boot_sector + 22),
        .sectors_per_track = *(u16 *)(boot_sector + 24),
        .num_heads = *(u16 *)(boot_sector + 26),
        .hidden_sectors = *(u32 *)(boot_sector + 28),
        .sector_count = *(u16*)(boot_sector + 19)
    };

    if (boot_sector[0x13] == 0) {
        volume->metadata.sector_count = *(u32 *)(boot_sector + 0x20);
    }

    volume->device = device;

    return 0; // Successfully mounted
}

int get_fat(FATVolume *volume, u8 *fat) {
    u8 fat_sector = volume->metadata.reserved_sectors;

    volume->device->read_sector(volume->device, fat_sector, volume->metadata.sectors_per_fat, fat);

    return 0;
}

void short_name_to_long_name(char name[8], char ext[3], char long_name[13*2]) {
    memset(long_name, 0, 13*2);

    u8 long_name_idx = 0;
    
    for (u8 i = 0; i < 8; i++) {
        if (name[i] == 0) break;

        long_name[long_name_idx] = name[i];
        long_name_idx += 2;
    }

    if (ext[0] == 0) return;

    long_name[long_name_idx] = '.';
    long_name_idx += 2;

    for (u8 i = 0; i < 3; i++) {
        if (ext[i] == 0) break;

        long_name[long_name_idx] = ext[i];
        long_name_idx += 2;
    }
}

int delete_entry(FATVolume *volume, char name[8], char ext[3]) {
    return 0;
}

fat12_directory_t generate_entry(char name[8], char ext[3], u8 flags) {
    fat12_directory_t entry = {
        .entry = {
            .attributes = flags,
            .reserved = 0,
            .creation_time = 0, // TODO
            .creation_date = 0, // TODO
            .last_access_date = 0, // TODO
            .first_cluster_high = 0,
            .last_modification_time = 0, // TODO
            .last_modification_date = 0, // TODO
            .first_cluster_low = 0,
            .file_size = 0
        },
        .long_name = {
            .order = 0x41,
            .attributes = 0x0F,
            .type = 0,
            .first_cluster_low = 0,
        }
    };

    memcpy(entry.entry.filename, name, 8);
    memcpy(entry.entry.extension, ext, 3);

    u16 long_name[13];
    short_name_to_long_name((char *)entry.entry.filename, (char *)entry.entry.extension, (char *)long_name);

    memcpy(entry.long_name.name1, long_name, sizeof(u16) * 5);
    memcpy(entry.long_name.name2, long_name, sizeof(u16) * 6);
    memcpy(entry.long_name.name3, long_name, sizeof(u16) * 2);

    return entry;
}

int create_entry(FATVolume *volume, char name[8], char ext[3], u8 flags) {
    fat12_directory_t entry = generate_entry(name, ext, flags);

    char sector[volume->metadata.bytes_per_sector * volume->metadata.sectors_per_cluster];

    u16 entry_location = 0;

    volume->device->read_sector(volume->device, volume->cwd, 1, sector);

    for (int j = 0; j < volume->metadata.bytes_per_sector * volume->metadata.sectors_per_cluster / 64; j++) {
        if (sector[j*64] == 0 || sector[j*64] == 0xE5) {
            entry_location = j*64;
            break;
        }
    }

    memcpy(&sector[entry_location], &entry, sizeof(entry));

    return volume->device->write_sector(volume->device, volume->cwd, 1, sector);
}

int get_entry(FATVolume *volume, char name[8], char ext[3]) {
    return 0;
}

int read_entry(FATVolume *volume, char name[8], char ext[3]) {
    return 0;
}

int write_entry(FATVolume *volume, char name[8], char ext[3]) {
    return 0;
}
