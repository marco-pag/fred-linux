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

#include <stdlib.h>

#include "scheduler.h"
#include "slot.h"
#include "../utils/logger.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

//#define RCFG_CHECK

//---------------------------------------------------------------------------------------------

static inline
int start_slot_after_rcfg_(struct scheduler *self, struct accel_req *request_done);

static inline
int start_rcfg_(struct scheduler *self, struct accel_req *request);

static inline
int push_req_fri_queue_(struct scheduler *self, struct accel_req *request);

//---------------------------------------------------------------------------------------------

static inline
int start_slot_after_rcfg_(struct scheduler *self, struct accel_req *request_done)
{
	int retval;
	struct slot *slot;
	struct accel_req *next_request;

	slot = accel_req_get_slot(request_done);
	assert(slot);

	// Start the hardware accelerator
	retval = slot_start_compute(slot, request_done);
	if (retval < 0)
		return -1;

	logger_log(LOG_LEV_FULL,"\tfred_sys: slot: %d of partition: %s"
							" started for hw-task: %s\n",
							slot_get_index(slot),
							partition_get_name(hw_task_get_partition(
								accel_req_get_hw_task(request_done))),
							hw_task_get_name(accel_req_get_hw_task(request_done)));

	// If the FRI queue is not empty start next reconfiguration
	if (!TAILQ_EMPTY(&self->fri_queue_head)) {

		// Get the head request from the FRI queue
		next_request = TAILQ_FIRST(&self->fri_queue_head);

		// Remove the head request from the FRI queue
		TAILQ_REMOVE(&self->fri_queue_head, next_request, queue_elem);

		logger_log(LOG_LEV_PEDANTIC,"\tfred_sys: FRI queue not empty\n");

		// Start reconfiguration
		retval = start_rcfg_(self, next_request);
		if (retval < 0)
			return -1;
	}

	return 0;
}


static inline
int start_rcfg_(struct scheduler *self, struct accel_req *request)
{
	int retval;

	// If the slot already contains the hw-task
	if (accel_req_get_skip_rcfg(request)) {
		logger_log(LOG_LEV_FULL,"\tfred_sys: skipping rcfg of slot: %d"
								" of partition: %s for hw-task: %s\n",
								slot_get_index(accel_req_get_slot(request)),
								partition_get_name(hw_task_get_partition(
										accel_req_get_hw_task(request))),
								hw_task_get_name(accel_req_get_hw_task(request)));

		// Start the slot immediately
		retval = start_slot_after_rcfg_(self, request);

	} else {
		logger_log(LOG_LEV_FULL,"\tfred_sys: start rcfg of slot:"
								"%d of partition: %s for hw-task: %s\n",
								slot_get_index(accel_req_get_slot(request)),
								partition_get_name(hw_task_get_partition(
										accel_req_get_hw_task(request))),
								hw_task_get_name(accel_req_get_hw_task(request)));

		// Start FPGA reconfiguration and bind hw-task to the slot
		slot_prepare_for_rcfg(accel_req_get_slot(request));
		retval = devcfg_start_prog(self->devcfg, request);
	}

	return retval;
}

static inline
void ins_req_ordered_(struct accel_req_queue *queue_head, struct accel_req *new_request)
{
	struct accel_req *req = NULL;

	// If queue is empty
	if (TAILQ_EMPTY(queue_head)) {
		TAILQ_INSERT_HEAD(queue_head, new_request, queue_elem);
		return;
	}

	// Fri queue contains at least one element
	TAILQ_FOREACH(req, queue_head, queue_elem) {
		if (accel_req_compare_timestamps(req, new_request) == 1)
			break;
	}

	// Last
	if (req == NULL)
		TAILQ_INSERT_TAIL(queue_head, new_request, queue_elem);
	else
		TAILQ_INSERT_BEFORE(req, new_request, queue_elem);
}

//TODO: integrate the function above to avoid insert and remove
static inline
int push_req_fri_queue_(struct scheduler *self, struct accel_req *request)
{
	// Insert the request directly into FRI queue
	ins_req_ordered_(&self->fri_queue_head, request);

	// If the inserted request is on top of FRI queue
	// and the DEVCFG is IDLE (not programming)
	if (devcfg_is_idle(self->devcfg) &&
		TAILQ_FIRST(&self->fri_queue_head) == request) {

		logger_log(LOG_LEV_PEDANTIC,"\tfred_sys: DevCfg idle & request on top\n");

		// Remove the head request from the FRI queue
		TAILQ_REMOVE(&self->fri_queue_head, request, queue_elem);

		// Start reconfiguration
		return start_rcfg_(self, request);
	}

	return 0;
}

//---------------------------------------------------------------------------------------------

int sched_init(struct scheduler **self, struct devcfg *devcfg)
{
	assert(devcfg);

	// Allocate and set everything to 0
	*self = calloc(1, sizeof(**self));
	if (!(*self))
		return -1;

	(*self)->devcfg = devcfg;

	// Initialize partition queues heads
	for (int i = 0; i < MAX_PARTITIONS; ++i)
		TAILQ_INIT(&((*self)->part_queues_heads[i]));

	// And fri queue head
	TAILQ_INIT(&(*self)->fri_queue_head);

	return 0 ;
}

void sched_free(struct scheduler *self)
{
	if(self)
		free(self);
}

// Acceleration request from software tasks
int sched_push_accel_req(struct scheduler *self, struct accel_req *request)
{
	int retval;
	int rcfg;
	struct hw_task *hw_task;
	struct slot *slot;
	struct partition *partition;

	assert(self);
	assert(request);

	// Set request's time stamp
	accel_req_stamp_timestamp(request);

	// Search a free slot in the partition
	hw_task = accel_req_get_hw_task(request);
	partition = hw_task_get_partition(hw_task);
	rcfg = partition_search_slot(partition, &slot, hw_task);

	// If all slots in the partition are occupied
	if (!slot) {
		logger_log(LOG_LEV_FULL,"\tfred_sys: all slots are busy for hw-task %s"
								", insert into partition %s queue\n",
								hw_task_get_name(hw_task),
								partition_get_name(partition));

		// Insert the new request into the partition FIFO queue
		TAILQ_INSERT_TAIL(&self->part_queues_heads[partition_get_index(partition)],
							request, queue_elem);

		retval = 0;

	// At least one free slot in the partition
	} else {
		// Reserve the slot for the HW-task
		slot_set_reserved(slot);
		accel_req_set_slot(request, slot);

		// If possible, set for skipping rcfg
		if (!rcfg)
			accel_req_set_skip_rcfg(request);

		logger_log(LOG_LEV_FULL,"\tfred_sys: hw-task: %s got slot: %d of"
								" its partition: %s, inserted in fri queue\n",
								hw_task_get_name(hw_task),
								slot_get_index(slot),
								partition_get_name(partition));

		// Push request into FRI queue. If request goes on top of FRI queue
		// and devcfg is idle start reconfiguration immediately
		retval = push_req_fri_queue_(self, request);
	}

	return retval;
}

// Reconfiguration done
int sched_rcfg_complete(struct scheduler *self, struct accel_req *request_done)
{
	struct slot *slot;
	uint32_t rcfg_time_us;

	assert(self);
	assert(request_done);

	// Get the slot that has been reconfigured
	slot = accel_req_get_slot(request_done);
	assert(slot);

	// Clear devcfg event
	rcfg_time_us = devcfg_clear_evt(self->devcfg);

	logger_log(LOG_LEV_FULL,"\tfred_sys: event: devcfg, slot: %d of partition: %s"
							" rcfg completed for hw-task: %s in %u us\n",
							slot_get_index(slot),
							partition_get_name(hw_task_get_partition(
								accel_req_get_hw_task(request_done))),
							hw_task_get_name(accel_req_get_hw_task(request_done)),
							rcfg_time_us);

	// Re-enable slot after it has been reconfigured
	slot_reinit_after_rcfg(slot);

#ifdef RCFG_CHECK
	// Check if the right hw-task has been reconfigured
	if (!slot_check_hw_task_consistency(slot)) {
		ERROR_PRINT("\tfred_sys: critical error: mismatch on slot %d"
					" of partition %s for hw-task %s\n",
					slot_get_index(slot),
					partition_get_name(hw_task_get_partition(
						accel_req_get_hw_task(request_done))),
					hw_task_get_name(accel_req_get_hw_task(request_done)));

		return -1;
	}
#endif

	// Start the hardware accelerator
	return start_slot_after_rcfg_(self, request_done);
}

// Hardware task execution completed
int sched_slot_complete(struct scheduler *self, struct accel_req *request_done)
{
	int retval;
	struct slot *slot;
	struct partition *partition;
	struct accel_req *request;
	struct accel_req_queue *part_queue_head;

	assert(self);
	assert(request_done);

	slot = accel_req_get_slot(request_done);
	assert(slot);

	partition = hw_task_get_partition(accel_req_get_hw_task(request_done));
	assert(partition);

	logger_log(LOG_LEV_FULL,"\tfred_sys: slot: %d of partition: %s"
							" completed execution of hw-task: %s\n",
							slot_get_index(slot), partition_get_name(partition),
							hw_task_get_name(accel_req_get_hw_task(request_done)));

	// Clear slot device
	slot_clear_after_compute(slot);

	// Notify the client
	retval = accel_req_notify_action(request_done);
	if (retval)
		return retval;

	// Check if there are pending requests in the partition queue (queue not empty)
	part_queue_head = &self->part_queues_heads[partition_get_index(partition)];
	if (!TAILQ_EMPTY(part_queue_head)) {

		// Get the head request from the partition FIFO queue
		request = TAILQ_FIRST(part_queue_head);

		// Remove the request from the partition FIFO queue
		TAILQ_REMOVE(part_queue_head, request, queue_elem);

		// Reserve the slot for the HW-task
		slot_set_reserved(slot);
		accel_req_set_slot(request, slot);

		// Push request into FRI queue. If request goes on top of FRI queue
		// and devcfg is idle start reconfiguration immediately
		retval = push_req_fri_queue_(self, request);
	}

	return retval;
}
