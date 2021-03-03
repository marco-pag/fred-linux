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

#ifndef FD_UTILS_H_
#define FD_UTILS_H_

int fd_utils_create_socket_pair(int *fd_0, int *fd_1);

int fd_utils_byte_write(int fd);

int fd_utils_byte_read(int fd);

int fd_utils_set_fd_nonblock(int fd);

#endif /* FD_UTILS_H_ */
