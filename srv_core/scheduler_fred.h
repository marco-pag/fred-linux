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

#ifndef SCHEDULER_FRED_H_
#define SCHEDULER_FRED_H_

#include <sys/queue.h>

#include "../parameters.h"
#include "accel_req.h"
#include "devcfg.h"
#include "scheduler.h"


enum sched_fred_mode {
    SCHED_FRED_NORMAL,
    SCHED_FRED_ALWAYS_RCFG
};

//---------------------------------------------------------------------------------------------

struct scheduler_fred {
    // ------------------------//
    struct scheduler scheduler;
    // ------------------------//

    enum sched_fred_mode mode;

    // Partitions queues heads
    struct accel_req_queue part_queues_heads[MAX_PARTITIONS];

    // Reconfiguration device queue
    // Partition queue head
    struct accel_req_queue fri_queue_head;

    // Reconfiguration device
    struct devcfg *devcfg;
};


//---------------------------------------------------------------------------------------------

int sched_fred_init(struct scheduler **self, enum sched_fred_mode mode, struct devcfg *devcfg);

//---------------------------------------------------------------------------------------------

#endif /* SCHEDULER_FRED_H_ */
