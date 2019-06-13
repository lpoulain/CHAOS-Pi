#ifndef __DISK_H
#define __DISK_H

#include "libc.h"

#define DIR_FILENAME		0
#define DIR_FILEEXT			8
#define DIR_ATTRIBUTES		11
#define DIR_FIRST_CLUSTER	26
#define DIR_SIZE			28

#define MBR_BYTES_PER_SECTOR			11
#define MBR_SECTORS_PER_CLUSTER			13
#define MBR_NB_RESERVED_SECTORS			14
#define MBR_NB_FAT_COPIES				16
#define MBR_NB_SECTORS_PER_FAT			36
#define MBR_FIRST_CLUSTER_OF_ROOTDIR	44
#define MBR_PARTITION1_START_SECTOR		454
#define MBR_SIGNATURE					510

#define BOOT_RECORD_SIGNATURE			0xAA55
/*
// The structures. However, using __packed__ does not work when running on a physical Raspberry Pi
// (alignment issues happen). So we're going the ugly way, loading the type as a blob and accessing
// its members through functions such as get_uint8(&mbr, MBR_NB_FAT_COPIES)

typedef struct __attribute__((__packed__, aligned(1))) {
    uint8 bootstrap[3];
    uint8 oem[8];
    uint16 bytes_per_sector;
    uint8 sectors_per_cluster;
    uint16 nb_reserved_sectors;
    uint8 nb_fat_copies;
    uint16 nb_root_dir_entries;
    uint16 nb_fs_sectors;
    uint8 media_type;
    uint16 sectors_per_fat_obsolete;
    uint16 nb_sectors_per_track;
    uint16 nb_heads;
    uint32 nb_hidden_sectors;
    uint32 nb_sectors_in_fs;
    uint32 sectors_per_fat;
    uint16 mirror_flags;
    uint16 fs_version;
    uint32 first_cluster_of_rootdir;
    uint16 info_sector_nb;
    uint16 backup_boot_sector;
    uint8 reserved1[12];
    uint8 logical_drive_nb;
    uint8 reserved2;
    uint8 ext_signature;
    uint32 partition_serial_number;
    uint8 volume_label[11];
    uint8 fs_type[8];
    uint8 blank[356];

	uint8 bootIndicator1;
	uint8 CHS_start1[3];
	uint8 partition_type1;
	uint8 CHS_end1[3];
	uint32 start_sector1;
	uint32 end_sector1;

	uint8 bootIndicator2;
	uint8 CHS_start2[3];
	uint8 partition_type2;
	uint8 CHS_end2[3];
	uint32 start_sector2;
	uint32 end_sector2;

	uint8 bootIndicator3;
	uint8 CHS_start3[3];
	uint8 partition_type3;
	uint8 CHS_end3[3];
	uint32 start_sector3;
	uint32 end_sector3;

	uint8 bootIndicator4;
	uint8 CHS_start4[3];
	uint8 partition_type4;
	uint8 CHS_end4[3];
	uint32 start_sector4;
	uint32 end_sector4;

    uint16 signature;
} BootRecord;

*/

// This is simular to the fat_DirEntry
// (same size), but with slight differences
typedef struct {
	uint8 data[32];
} DirEntry;

typedef struct {
//	DirEntry info;
	uint8 attributes;
	uint first_cluster;
	uint size;
	char filename[256];
	unsigned char *body;
} File;

void disk_load_mbr();
File *disk_ls(uint cluster);
int disk_cd(unsigned char *dir_name, DirEntry *dir_index, char *path);
void disk_load_file_index();
void disk_get_filename(DirEntry *f, unsigned char *filename);
uint8 disk_has_long_filename(DirEntry *f);
char *disk_get_long_filename(DirEntry *f);
uint8 disk_is_dir_entry_valid(File *f);
int disk_load_file(const char *filename, uint dir_cluster, DirEntry *dir_index, File *f);
int disk_write_file(File *f);
uint8 disk_skip_n_entries(DirEntry *f);
uint8 disk_is_directory(File *f);

#define DISK_CMD_OK				-1
#define DISK_ERR_DOES_NOT_EXIST	-2
#define DISK_ERR_NOT_A_DIR		-3
#define DISK_ERR_NOT_A_FILE		-4
#define DISK_FILE_EMPTY			-5

#define ROOT_DIR_CLUSTER		2

#endif
