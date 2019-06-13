#include "libc.h"
#include "heap.h"
#include "disk.h"
#include "FAT32.h"
#include "sd.h"
#include "uart.h"
#include "kheap.h"

uint32 fat32_partition_start_sector;

uint16 fat32_bytes_per_sector;
uint8 fat32_sectors_per_cluster;
uint16 fat32_nb_reserved_sectors;
uint8 fat32_nb_fat_copies;
uint32 fat32_sectors_per_fat;
uint32 fat32_first_cluster_of_rootdir;

extern unsigned char _end;

uint8 boot_record[512];

void FAT32_load_partition_mbr(uint32 partition_start_sector) {
	fat32_partition_start_sector = partition_start_sector;

	sd_readblock(partition_start_sector, &boot_record, 1);

	fat32_bytes_per_sector = get_uint16(&boot_record, MBR_BYTES_PER_SECTOR);
	fat32_sectors_per_cluster = get_uint8(&boot_record, MBR_SECTORS_PER_CLUSTER);
	fat32_nb_reserved_sectors = get_uint16(&boot_record, MBR_NB_RESERVED_SECTORS);
	fat32_nb_fat_copies = get_uint8(&boot_record, MBR_NB_FAT_COPIES);
	fat32_sectors_per_fat = get_uint32(&boot_record, MBR_NB_SECTORS_PER_FAT);
	fat32_first_cluster_of_rootdir = get_uint32(&boot_record, MBR_FIRST_CLUSTER_OF_ROOTDIR);
	uint16 fat32_signature = get_uint16(&boot_record, MBR_SIGNATURE);

    uart_puts("\nNb bytes per sector: 0x");
    uart_hex(fat32_bytes_per_sector);

    uart_puts("\nNb reserved sectors: 0x");
    uart_hex(fat32_nb_reserved_sectors);

    uart_puts("\nNb sectors per cluster: 0x");
    uart_hex(fat32_sectors_per_cluster);

    uart_puts("\nFirst cluster of root dir: 0x");
    uart_hex(fat32_first_cluster_of_rootdir);

    uart_puts("\nSignature: 0x");
    uart_hex(fat32_signature);

    uart_puts("\nNb sectors per FAT: 0x");
    uart_hex(fat32_sectors_per_fat);

    uart_puts("\nNb of FAT blocks: 0x");
    uart_hex(fat32_nb_fat_copies);
}

void FAT32_load_table() {
//	memset(&FAT_table, 0, 200000*4);
//	read_sectors(mbr.nb_reserved_sectors, &FAT_table, mbr.sectors_per_fat);

	// Read the FAT table

/*	for (int i=0; i<8; i++) {
		read_sector(buf + 512*i, i);
		buf += 512;
	}*/
}

uint32 FAT32_read_entry(uint idx) {
	return 0; //FAT_table[idx];
/*	uint offset = idx / 2;

	unsigned char *fat_raw = (unsigned char*)FAT_table + offset*3;

	// Convert it to an array
	uint16 fat_entry;

	if (idx % 2 == 0) {
		fat_entry = (uint16)fat_raw[0] + ((uint16)(fat_raw[1] & 0x0F) << 8);
	} else {
		fat_entry = (((uint16)fat_raw[2]) << 4) + ((uint16)(fat_raw[1] & 0xF0)) / 16;
	}

	return fat_entry;*/
}

void FAT32_read_file(DirEntry *f, char *buf) {
	void FAT32_load_table();

	uint16 fat_entry = get_uint16(f, DIR_FIRST_CLUSTER);

//	printf("Sector: %d\n", fat_entry*8 + 32);
	while (fat_entry != 0 && fat_entry < (uint16)0xFF) {
		for (int i=0; i<8; i++) {
//			read_sector(buf, fat_entry*8 + 32 + i);
			buf += 512;
		}
		fat_entry = FAT32_read_entry(fat_entry);
//		debug_i("FAT entry: ", fat_entry);
	}
}

void FAT32_write_file(DirEntry *f, char *buf) {
	void FAT32_load_table();

	uint16 fat_entry = get_uint16(f, DIR_FIRST_CLUSTER);
	while (fat_entry != 0 && fat_entry < (uint16)0xFF) {
		for (int i=0; i<8; i++) {
//			printf("Writing sector %d\n", fat_entry*8 + 32 + i);
//			write_sector(buf, fat_entry*8 + 32 + i);
			buf += 512;
			for (int i=0; i<1000000; i++);
		}
		fat_entry = FAT32_read_entry(fat_entry);
//		printf("FAT entry: %d\n", fat_entry);
	}
}

// Reads the directory (DirEntry format) and converts the entries
// into File format
File *FAT32_read_directory(uint cluster) {
	DirEntry *dir_index = (DirEntry*)kmalloc_pages(1, "FAT32 directory index");
	File *files = (File*)kmalloc_pages(7, "File directory index");

	clear_pages(dir_index, 1);
	clear_pages(files, 7);

	uint dir_nb = 0, file_nb = 0;

	// Loads the directory sectors from disk
	sd_readblock(fat32_partition_start_sector + fat32_nb_reserved_sectors + fat32_nb_fat_copies * fat32_sectors_per_fat + cluster * fat32_sectors_per_cluster, dir_index, 8);

	// Converts the fat_DirEntry records info DirEntry
	// - Move the attribute to the first byte
	// - Copy the 
	// - Compute the long name if any
	int LFN_entry_end = -1;
	char c;
	int filename_len;
	char *str;

	for (dir_nb=0; dir_nb < 100; dir_nb++) {

		uint8 *fat_filename = get_ptr(&dir_index[dir_nb], DIR_FILENAME);
		uint8 *fat_fileext = get_ptr(&dir_index[dir_nb], DIR_FILEEXT);
		uint8 fat_attributes = get_uint8(&dir_index[dir_nb], DIR_ATTRIBUTES);

		if (fat_filename[0] == 0 ||		// unallocated
			fat_filename[0] == 0xE5) {	// deleted
				continue;
		}

		// If this is an entry that stores a long name
		// remember where it is and skip this entry
		if ((fat_attributes & 0xF) == 0xF) {
			if (LFN_entry_end < 0) LFN_entry_end = dir_nb;
			continue;
		}

		files[file_nb].attributes = fat_attributes;
		files[file_nb].first_cluster = get_uint16(&dir_index[dir_nb], DIR_FIRST_CLUSTER);
		files[file_nb].size = get_uint32(&dir_index[dir_nb], DIR_SIZE);

		// If the file has a long name, compute it
		if (LFN_entry_end >= 0) {
			filename_len = 0;

			// We need to go back the entries to reconstruct the name
			// The n-1 last LFN entries are to be taken completely
			for (int j=dir_nb-1; j>LFN_entry_end; j--) {
				str = (char*)&dir_index[j];
				for (int k=1; k<32; k+= 2) {
					// We skip a few bytes
					if (k == 11) k += 3;
					if (k == 26) k += 2;
					files[file_nb].filename[filename_len++] = str[k];
				}
			}

			// The last LFN entry may not be complete
			str = (char*)&dir_index[LFN_entry_end];
			for (int k=1; k<32 && str[k] != 00; k+= 2) {
				if (k == 11) k += 3;
				if (k == 26) k += 2;
				files[file_nb].filename[filename_len++] = str[k];
			}

			files[file_nb].filename[filename_len] = 0;

			LFN_entry_end = -1;			
		}
		// Computes the 8.3 filename in a print-ready format
		else {
			for (filename_len=0; filename_len<8 && fat_filename[filename_len] != 0x20; filename_len++) {
				c = fat_filename[filename_len];
				if (c >= 'A' && c <= 'Z') files[file_nb].filename[filename_len] = c - 'A' + 'a';
				else files[file_nb].filename[filename_len] = c;
			}

			if (fat_fileext[0] == 0x20) files[file_nb].filename[filename_len] = 0;
			else {
				files[file_nb].filename[filename_len++] = '.';
				for (int j=0; j<3 && fat_fileext[j] != 0x20; j++) {
					c = fat_fileext[j];
					if (c >= 'A' && c <= 'Z') files[file_nb].filename[filename_len++] = c - 'A' + 'a';
					else files[file_nb].filename[filename_len++] = c;
				}
				files[file_nb].filename[filename_len] = 0;
			}
		}

		file_nb++;
	}

	return files;
}
