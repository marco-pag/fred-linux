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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "fd_utils.h"
#include "../utils/dbg_print.h"

int fd_utils_create_socket_pair(int *fd_0, int *fd_1)
{
    int retval;
    int fds[2];

    retval = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    if (retval)
        return -1;

    *fd_0 = fds[0];
    *fd_1 = fds[1];

    return 0;
}

int fd_utils_byte_write(int fd)
{
    int retval;
    char data = 'a';

    retval = write(fd, &data, sizeof(data));
    if (retval != 1)
        return -1;

    return 0;
}

int fd_utils_byte_read(int fd)
{
    int retval;
    char data;

    retval = read(fd, &data, sizeof(data));
    if (retval != 1)
        return -1;

    return 0;
}

int fd_utils_set_fd_nonblock(int fd)
{
    int flags;
    int retval;

    if (fd < 0)
        return -1;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        return -1;

    flags |= O_NONBLOCK;
    retval = fcntl(fd, F_SETFL, flags);
    if (retval < 0)
        return -1;

    return 0;
}
