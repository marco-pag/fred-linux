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

#ifndef FRED_MSG_H_
#define FRED_MSG_H_

#include <stdint.h>

//-------------------------------------------------------------------------------

enum msg_head_ {
    // Client requests
    FRED_MSG_INIT       = 101,
    FRED_MSG_BIND       = 201,
    FRED_MSG_RUN        = 301,
    // Server Replies
    FRED_MSG_DONE       = 401,
    FRED_MSG_OVERRUN    = 402,
    FRED_MSG_ACK        = 501,
    FRED_MSG_BUFFS      = 601,
    FRED_MSG_ERROR      = 701,  // Client request error
    // Server notices
    FRED_MSG_CRIT       = 801,  // Server internal error
};

struct fred_msg {
    enum msg_head_ head;
    uint32_t arg;
};

//-------------------------------------------------------------------------------

static inline
int fred_msg_get_head(const struct fred_msg *msg)
{
    return msg->head;
}

static inline
void fred_msg_set_head(struct fred_msg *msg, int head)
{
    msg->head = head;
}

static inline
uint32_t fred_msg_get_arg(const struct fred_msg *msg)
{
    return msg->arg;
}

static inline
void fred_msg_set_arg(struct fred_msg *msg, uint32_t arg)
{
    msg->arg = arg;
}

//-------------------------------------------------------------------------------

#endif /* FRED_MSG_H_ */
