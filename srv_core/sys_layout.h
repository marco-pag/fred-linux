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

#ifndef SYS_LAYOUT_H_
#define SYS_LAYOUT_H_

#include "../parameters.h"
#include "partition.h"
#include "reactor.h"
#include "../srv_support/buffctl.h"
#include "../hw_support/sys_hw_config.h"
#include "scheduler.h"

//---------------------------------------------------------------------------------------------

struct sys_layout {
    struct partition *partitions[MAX_PARTITIONS];
    int partitions_count;

    struct hw_task *hw_tasks[MAX_HW_TASKS];
    int hw_tasks_count;

    buffctl_ft *buffctl;
};

//---------------------------------------------------------------------------------------------

int sys_layout_init(struct sys_layout **self, const struct sys_hw_config *hw_config,
                    const char *arch_file, const char *hw_tasks_file,
                    struct scheduler *scheduler, buffctl_ft *buffctl);

void sys_layout_free(struct sys_layout *self);

struct hw_task *sys_layout_get_hw_task(const struct sys_layout *self,
                                        uint32_t hw_task_id);

int sys_layout_get_hw_tasks(const struct sys_layout *self, struct hw_task **hw_tasks);

int sys_layout_register_slots(struct sys_layout *self, struct reactor *reactor);

void sys_layout_print(const struct sys_layout *self);

//---------------------------------------------------------------------------------------------

#endif /* SYS_LAYOUT_H_ */
