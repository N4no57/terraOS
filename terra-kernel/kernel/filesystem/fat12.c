#include "../headers/utils.h"

typedef struct __attribute__((packed)) {
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
    fat12_header_t header; // FAT12 header
    u8 *fat; // pointer to fat
    // what other information is necessary?
    // I'll figure it out in a minute
} fat12_filesystem_t;


