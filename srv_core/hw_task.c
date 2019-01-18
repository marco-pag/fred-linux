/*
 * Fred for Linux. Experimental support.
 *
 * Copyright (C) 2018, Marco Pagani, ReTiS Lab.
 * <marco.pag(at)outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "hw_task.h"
#include "../shared_user/fred_buff.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

static
uint32_t swab32_(uint32_t x)
{
	return x << 24 | x >> 24 |
			(x & (uint32_t)0x0000ff00UL) << 8 |
			(x & (uint32_t)0x00ff0000UL) >> 8;
}

// Straight from Xilinx's code. Returns the new size.
static
size_t mangle_bitstream_(uint8_t *bitstream, size_t length)
{
	int i;
	int endian_swap = 0;
	uint32_t *bs_wrd;

	// First block contains a header
	if (length > 4) {
		// Look for sync word
		for (i = 0; i < length - 4; i++) {
			if (memcmp(bitstream + i, "\x66\x55\x99\xAA", 4) == 0) {
				DBG_PRINT("fred_sys: bitstream: found normal sync word\n");
				endian_swap = 0;
				break;
			}
			if (memcmp(bitstream + i, "\xAA\x99\x55\x66", 4) == 0) {
				DBG_PRINT("fred_sys: bitstream: found swapped sync word\n");
				endian_swap = 1;
				break;
			}
		}

		// Remove the header, aligning the data on word boundary
		if (i != length - 4) {
			length -= i;
			DBG_PRINT("fred_sys: bitstream: removing the header\n");
			memmove(bitstream, bitstream + i, length);
		}
	}

	// Fixup endianess of the data
	if (endian_swap) {
		for (i = 0; i < length; i += 4) {
			bs_wrd = (uint32_t *)&bitstream[i];
			*bs_wrd = swab32_(*bs_wrd);
		}
		DBG_PRINT("fred_sys: bitstream: endianess fixed\n");
	}

	// Convert number of bytes to number of words.
	if (length % 4)
		return (length / 4 + 1);
	else
		return length / 4;
}

//---------------------------------------------------------------------------------------------

// Helper function. Take advantage of the same code for mapping data buffer
// to map bistream buffer for bitstream loading
static
int gen_user_buffs_(struct fred_buff_if *buff_if, struct fred_user_buff *buff_usr)
{
	char dev_usr_name[MAX_PATH];

	assert(buff_if);
	assert(buff_usr);

	// Convert device name (from kernel mod) into user form
	// es: "fred!buffN" -> "/dev/fred/buffN"
	sprintf(dev_usr_name,"/dev/%s", buff_if->dev_name);
	dev_usr_name[strcspn(dev_usr_name, "!")] = '/';

	strncpy(buff_usr->dev_name, dev_usr_name, MAX_PATH - 1);
	buff_usr->length = buff_if->length;

	return 0;
}

// NOTE: this part will change with the new reconfiguration driver
static
ssize_t load_bit_buffer_dev_(buffctl_ft *buffctl, char *file_name, struct fred_buff_if **buff_if)
{
	int retval;
	ssize_t length;
	void *buff_v;
	struct fred_user_buff user_buffer;

	unsigned int b_read;
	unsigned int file_size;
	FILE *file_p;

	file_p = fopen(file_name, "r");
	if (!file_p) {
		DBG_PRINT("fred_sys: could not open bitstream file %s\n", file_name);
		return -1;
	}

	// Get file length
	fseek(file_p , 0 , SEEK_END);
	file_size = ftell(file_p);
	rewind(file_p);

	// Alloc bistream buffer device
	retval = buffctl_alloc_buff(buffctl, buff_if, file_size);
	if (retval) {
		DBG_PRINT("fred_sys: could not allocate buffer for bitstream %s\n", file_name);
		return -1;
	}

	// Reuse the code for mapping user buffer to fill the buffer
	// associated with the device file
	fred_buff_init(&user_buffer);
	gen_user_buffs_(*buff_if, &user_buffer);

	// Map bistream buffer into server process virtual address space
	buff_v = fred_buff_map(&user_buffer);

	// Read bitstream
	b_read = fread(buff_v, 1, file_size, file_p);
	fclose(file_p);

	if (b_read != file_size) {
		DBG_PRINT("fred_sys: size mismatch for bitstream %s\n", file_name);
		return -1;
	}

	// Mangle returns the size for the xdevcfg
	length = mangle_bitstream_((uint8_t *)fred_buff_if_get_phy_addr(*buff_if),
								fred_buff_if_get_lenght(*buff_if));

	// Unmap bistream buffer from server process virtual address space
	fred_buff_unmap(&user_buffer);

	return length;
}

//---------------------------------------------------------------------------------------------

int hw_task_add_buffer(struct hw_task *self, unsigned int buff_size)
{
	assert(self);

	DBG_PRINT("fred_sys: setting buffer %u of size %u for HW-task %s\n",
				self->data_buffs_count, buff_size, self->name);

	self->data_buffs_sizes[self->data_buffs_count++] = buff_size;

	return 0;
}

void hw_task_print(const struct hw_task *self, char *str, int str_size)
{
	assert(self);

	snprintf(str, str_size, "hw-task %d : %s using %d buffers : partition %s",
				self->hw_id, self->name, self->data_buffs_count,
				partition_get_name(self->partition));
}

int hw_task_init(struct hw_task **self, uint32_t hw_id, const char *name,
					const char *bits_path, struct partition *partition,
					buffctl_ft *buffctl)
{
	ssize_t xdev_length;
	char bit_path[MAX_PATH];
	int bits_count;
	const char *part_name;

	// Allocate and set everything to 0
	*self = calloc(1, sizeof(**self));
	if (!(*self))
		return -1;

	// Set id, name and associate with the partition
	(*self)->hw_id = hw_id;
	strncpy((*self)->name, name, sizeof((*self)->name) - 1);
	(*self)->partition = partition;
	bits_count = partition_get_slots_count((*self)->partition);
	part_name = partition_get_name((*self)->partition);

	// One bitstream for each slot in the partition
	for (int i = 0; i < bits_count; ++i) {
		// Build bistream path with name
		sprintf(bit_path, "%s%s/%s/%s_s%u.bin",
				FRED_PATH, bits_path, part_name, (*self)->name, i);

		// Load bitstream
		xdev_length = load_bit_buffer_dev_(buffctl, bit_path, &((*self)->bits_buffs[i]));
		if (xdev_length < 0) {
			DBG_PRINT("fred_sys: error while reading bit file: %s\n", bit_path);
			return -1;
		}

		// Fill buffer for xdevcfg with buffers coordinates
		(*self)->bits_phys[i].length = xdev_length;
		(*self)->bits_phys[i].phy_addr = fred_buff_if_get_phy_addr((*self)->bits_buffs[i]);

		DBG_PRINT("fred_sys: loaded slot %u bitstream for hw-task %s, size: %u\n",
				i, (*self)->name, (*self)->bits_phys[i].length);

	}

	return 0;
}

void hw_task_free(struct hw_task *self, buffctl_ft *buffctl)
{
	int bits_count;

	if (!self)
		return;

	bits_count = partition_get_slots_count(self->partition);

	for (int i = 0; i < bits_count; ++i) {
		if (self->bits_buffs[i]) {
			buffctl_free_buff(buffctl, self->bits_buffs[i]);
		}
	}

	free(self);
}
