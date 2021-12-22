/*
 * Fred for Linux. Experimental support.
 *
 * Copyright (C) 2018-2021, Marco Pagani, ReTiS Lab.
 * <marco.pag(at)outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
*/

#include "buffctl.h"

#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

#include "../parameters.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

// TODO: move in parameters?
static const char default_dev[] = "/dev/fred/buffctl";

//---------------------------------------------------------------------------------------------

struct buffctl_ {
    int fd;
    char dev_name[MAX_PATH];
};

//---------------------------------------------------------------------------------------------

int buffctl_open(buffctl_ft **buffctl, const char *dev_name)
{

    *buffctl = calloc(1, sizeof(**buffctl));
    if (*buffctl == NULL) {
        ERROR_PRINT("buffctl: could not allocate memory\n");
        return -1;
    }

    strncpy((*buffctl)->dev_name,
            dev_name == NULL ? default_dev : dev_name,
            sizeof((*buffctl)->dev_name) - 1);

    (*buffctl)->fd = open((*buffctl)->dev_name, O_RDWR);
    if ((*buffctl)->fd < 0) {
        ERROR_PRINT("buffctl: failed to open %s \n", (*buffctl)->dev_name);
        free(*buffctl);
        return -1;
    }

    return 0;
}

int buffctl_close(buffctl_ft *buffctl)
{
    assert(buffctl);

    close(buffctl->fd);
    free(buffctl);
    return 0;
}

int buffctl_alloc_buff(buffctl_ft *buffctl, struct fred_buff_if **buff_if, size_t size)
{
    int retval;

    assert(buffctl);

    *buff_if = calloc(1, sizeof(**buff_if));
    if (*buff_if == NULL) {
        ERROR_PRINT("buffctl: could not allocate memory\n");
        return -1;
    }

    // Set requested size
    (*buff_if)->length = size;

    // Request a new buffer to the buffctl kernel module
    retval = ioctl(buffctl->fd, FRED_BUFFCTL_ALLOC, *buff_if);
    if (retval < 0) {
        ERROR_PRINT("buffctl: kernel module could not allocate a new buff\n");
        return -1;
    }

    return 0;
}

int buffctl_free_buff(buffctl_ft *buffctl, struct fred_buff_if *buff_if)
{
    int retval;

    assert(buffctl);

    // Ask the kernel module to free the buffer
    retval = ioctl(buffctl->fd, FRED_BUFFCTL_FREE, &buff_if->id);
    if (retval < 0) {
        ERROR_PRINT("buffctl: kernel module failed to free buffer\n");
        retval = -1;
    } else {
        retval = 0;
    }

    // free interface structure
    free(buff_if);

    return retval;
}
