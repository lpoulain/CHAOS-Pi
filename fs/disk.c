#include "libc.h"
#include "kheap.h"
#include "fat32.h"
#include "disk.h"
#include "sd.h"
#include "uart.h"

//extern void write_sector(unsigned char *buf, uint addr);
extern int read_sector(unsigned char *buf, uint addr);

extern unsigned char _end;

uint8 *mbr;

void disk_load_mbr() {
	mbr = kmalloc(512);

	sd_readblock(0, (unsigned char *)mbr, 1);
	uart_dump_mem(mbr, 512);

	uint start_sector = get_uint32(mbr, MBR_PARTITION1_START_SECTOR);
	uint16 signature = get_uint16(mbr, MBR_SIGNATURE);

	if (signature != BOOT_RECORD_SIGNATURE) {
		uart_puts("ERROR: invalid Master Boot Record\n");
		uart_dump_mem(mbr, 512);
		return;
	}

	FAT32_load_partition_mbr(start_sector);
}

uint8 disk_is_directory(File *f) {
	return (f->attributes & 0x10);
}

File *disk_ls(uint cluster) {
	return FAT32_read_directory(cluster);
}

uint8 disk_skip_n_entries(DirEntry *f) {
	/*
	if (f->attributes == 0 ||			// unallocated
		f->attributes == 0xE5)			// deleted
		return 1;

	if (f->attributes == 0xFF) return f->filename[0];
*/
	return 0;
}

int find_dir_entry(const char *filename_requested, DirEntry *dir_index) {
	/*
	int idx = 0, long_filename_idx;
	uint8 skip;

	for (idx=0; dir_index[idx].filename[0] != 0 && idx < 2048; idx++) {
		skip = disk_skip_n_entries(&dir_index[idx]);
		if (skip > 0) {
			idx += (skip - 1);
			continue;
		}

		long_filename_idx = dir_index[idx].has_longname;
		if (long_filename_idx != 0 && !strcmp( ((char*)&dir_index[idx - long_filename_idx]) + 2, filename_requested )) {
			return idx;
		}
		if (!strcmp(dir_index[idx].filename, filename_requested)) {
			return idx;
		}
	}
*/
	return -1;
}

int disk_cd(unsigned char *dir_name, DirEntry *dir_index, char *path) {
	/*
	int idx = find_dir_entry(dir_name, dir_index);

	// We check whether the directory exists and is a directory
	if (idx < 0) return DISK_ERR_DOES_NOT_EXIST;

	DirEntry *dir = &dir_index[idx];

	if (!disk_is_directory(dir)) return DISK_ERR_NOT_A_DIR;

	int i, len = strlen(path);
	if (!strcmp(dir_name, "..")) {
		int i;
		for (i=len; path[i] != '/' && i>=0; i--);
		path[i] = 0;
	} else if (strcmp(dir_name, ".")) {
		path[len] = '/';
		strcpy(path + len + 1, dir_name);
	}

	if (dir->address == 0) return 2;
	return dir->address + 4;
	*/

	return 0;
}

void disk_load_file_index() {
//	FAT32_load_mbr();
}

uint8 disk_is_dir_entry_valid(File *f) {
	if (f->attributes == 0 ||			// unallocated
		f->attributes == 0xE5)			// deleted
		return 0;

	return 1;
}
/*
uint8 disk_has_long_filename(File *f) {
	return (f->has_longname != 0);
}

char *disk_get_long_filename(File *f) {
	if (f->has_longname == 0) return 0;

	f -= f->has_longname;
	return ((char*)f) + 2;
}
*/
// Input: the filename and the cluster where the directory is
// Output: the directory index (that contains file metadata) and
// the File object
int disk_load_file(const char *filename, uint dir_cluster, DirEntry *dir_index, File *f) {
	/*
	// Loads all the directory entries from the current dir into dir_index
	disk_ls(dir_cluster, dir_index);
	// Look for the entry named "filename" in that index
	int idx = find_dir_entry(filename, dir_index);

	// We check whether the file exists
	if (idx < 0) return DISK_ERR_DOES_NOT_EXIST;

	DirEntry *entry = &dir_index[idx];

	// We check it is a file
	if (disk_is_directory(entry)) 	return DISK_ERR_NOT_A_FILE;

	if (entry->size > 0) {
		f->body = (char*)kmalloc_pages( ((entry->size / 0x1000) + 1), "File");
		FAT32_read_file(entry, f->body);
	}

	f->info = *entry;
	f->dir_entry_sector = dir_cluster * 8 + (idx * sizeof(DirEntry)) / 512 ;
	f->dir_entry_offset = (idx * sizeof(DirEntry)) % 512;

	if (disk_has_long_filename(entry)) strcpy(f->filename, disk_get_long_filename(entry));
	else strcpy(f->filename, entry->filename);
*/
	return DISK_CMD_OK;
}

int disk_write_file(File *f) {
	/*
	FAT32_write_file(&f->info, f->body);

	unsigned char buffer[512];
	read_sector((unsigned char *)&buffer, f->dir_entry_sector);

//	printf("Buffer: %x (sector %d, offset %d)\n", &buffer, f->dir_entry_sector, f->dir_entry_offset);
	fat_DirEntry *entry = (fat_DirEntry*)((uint)(&buffer) + f->dir_entry_offset);
	entry->size = f->info.size;
//	printf("[%x] Body: %x, Size: %d\n", entry, f->body, entry->size);

//	write_sector((unsigned char *)&buffer, f->dir_entry_sector);
*/
	return DISK_CMD_OK;
}
