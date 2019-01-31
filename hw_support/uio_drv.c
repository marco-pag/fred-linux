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

// for alphasort
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/mman.h>

#include "uio_drv.h"
#include "../parameters.h"
#include "../utils/dbg_print.h"

// Simple -single map- UIO device
struct uio_dev_ {
	// Device tree name and UIO number
	// UIO name: (uio<num>)
	uint32_t uio_num;
	char dt_name[MAX_NAMES];

	// Device registers
	uintptr_t regs_addr;
	size_t regs_size;

	// UIO device fd (when opened)
	int uio_fd;
	// Base address when mapped into process space
	uintptr_t map_base;
};

// TODO: add some range checks
static inline
uint32_t str_to_uint32_(const char *string)
{
	return (uint32_t)strtol(string, (char **)NULL, 10);
}

static inline
size_t strhex_to_size_(const char *string)
{
	return (size_t)strtol(string, (char **)NULL, 16);
}

static inline
uintptr_t strhex_to_uintptr_(const char *string)
{
	return (uintptr_t)strtol(string, (char **)NULL, 16);
}

static
int read_line_(char *f_name, char *linebuf)
{
	char *retc;
	FILE *file_p;

	file_p = fopen(f_name, "r");
	if (!file_p)
		return -1;

	retc = fgets(linebuf, MAX_NAMES, file_p);
	fclose(file_p);

	if (!retc)
		return -2;

	// Remove newline
	linebuf[strcspn(linebuf, "\n")] = 0;
	return 0;
}


int uio_dev_init(uio_dev_ft **uio_dev, const char* dev_name)
{
	struct dirent **namelist;
	int dirs;
	char* uio_name;
	char f_path[MAX_PATH];
	char uio_field[MAX_NAMES];
	int dev_found = 0;

	const char sys_class_uio[] = "/sys/class/uio";
	const char uio[] = "uio";

	*uio_dev = calloc(1, sizeof(**uio_dev));
	if (!(*uio_dev))
		return -1;

	// Scan class directory for UIO devices
	dirs = scandir(sys_class_uio, &namelist, NULL, alphasort);
	if (dirs < 0){
		ERROR_PRINT("uio_drv: unable to find class for UIO devices");
		free(uio_dev);
		return -1;
	}

	// For each device entry in the /sys/class/uio/ dir
	for (size_t i = 0;  i < dirs; ++i) {

		// Build name file path
		sprintf(f_path, "%s/%s/name", sys_class_uio, namelist[i]->d_name);

		// Read device name (devtree name) from file and check if it's requested device
		if (!read_line_(f_path, uio_field) && !strcmp(uio_field, dev_name)) {
			dev_found = 1;

			// Fill device name
			strcpy((*uio_dev)->dt_name, uio_field);

			// UIO number name
			uio_name = namelist[i]->d_name;
			// Fill UIO device number, get rid of the "uio" part of the name
			(*uio_dev)->uio_num = str_to_uint32_(&uio_name[strspn(uio_name, uio)]);

			// Read map address
			sprintf(f_path, "%s/uio%u/maps/map0/addr", sys_class_uio, (*uio_dev)->uio_num);
			read_line_(f_path, uio_field);
			(*uio_dev)->regs_addr = strhex_to_uintptr_(uio_field);

			// Read map size
			sprintf(f_path, "%s/uio%u/maps/map0/size", sys_class_uio, (*uio_dev)->uio_num);
			read_line_(f_path, uio_field);
			(*uio_dev)->regs_size = strhex_to_size_(uio_field);

			break;
		}
	}

	// Free namelist
	for (size_t i = 0;  i < dirs; ++i) {
		free(namelist[i]);
	}
	free(namelist);

	// Check if the device has been found
	if (!dev_found) {
		ERROR_PRINT("uio_drv: unable to find UIO device: %s\n", dev_name);
		free(uio_dev);
		return -1;
	}

	// Build path and open the device file
	sprintf(f_path, "/dev/uio%d", (*uio_dev)->uio_num);
	(*uio_dev)->uio_fd = open(f_path, O_RDWR);
	if ((*uio_dev)->uio_fd < 0) {
		ERROR_PRINT("uio_drv: unable to open UIO device: %s\n", dev_name);
		free(uio_dev);
		return -1;
	}

	// Map into the process memory space
	(*uio_dev)->map_base = (uintptr_t)mmap(	NULL, (*uio_dev)->regs_size,
											PROT_READ | PROT_WRITE, MAP_SHARED,
											(*uio_dev)->uio_fd, 0 * getpagesize());
	if (!(*uio_dev)->map_base) {
		ERROR_PRINT("uio_drv: unable to mmap UIO device: %s\n", dev_name);
		close((*uio_dev)->uio_fd);
		free(uio_dev);
		return -2;
	}

	return 0;
}

void uio_dev_free(uio_dev_ft *uio_dev)
{
	if (!uio_dev)
		return;

	// Unmap from the process memory space
	if (uio_dev->map_base)
		munmap((void *)uio_dev->map_base, uio_dev->regs_size);

	close(uio_dev->uio_fd);

	free(uio_dev);
}

uintptr_t uio_get_base_addr(const uio_dev_ft *uio_dev)
{
	assert(uio_dev);

	return uio_dev->map_base;
}

int uio_get_fd(const uio_dev_ft *uio_dev)
{
	assert(uio_dev);

	return uio_dev->uio_fd;
}

int uio_read_for_irq(uio_dev_ft *uio_dev)
{
	int32_t pending;

	assert(uio_dev);

	// block on the file waiting for the interrupt
	pending = 0;
	read(uio_dev->uio_fd, &pending, sizeof(pending));

	return pending;
}

void uio_clear_gic(uio_dev_ft *uio_dev)
{
	static const int32_t reenable = 1;

	assert(uio_dev);

	// re-enable the interrupt in the interrupt controller
	// thru the UIO subsystem
	write(uio_dev->uio_fd, &reenable, sizeof(reenable));
}
