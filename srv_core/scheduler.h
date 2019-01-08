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

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <sys/queue.h>

#include "../parameters.h"
#include "accel_req.h"
#include "devcfg.h"

//---------------------------------------------------------------------------------------------

struct scheduler {
	// Partitions queues heads
	struct accel_req_queue part_queues_heads[MAX_PARTITIONS];

	// Reconfiguration device queue
	// Partition queue head
	struct accel_req_queue fri_queue_head;

	// Reconfiguration device
	struct devcfg *devcfg;
};


//---------------------------------------------------------------------------------------------

int sched_init(struct scheduler **self, struct devcfg *devcfg);

void sched_free(struct scheduler *self);

int sched_push_accel_req(struct scheduler *self, struct accel_req *request);

int sched_rcfg_complete(struct scheduler *self, struct accel_req *request_done);

int sched_slot_complete(struct scheduler *self, struct accel_req *request_done);

//---------------------------------------------------------------------------------------------

#endif /* SCHEDULER_H_ */
