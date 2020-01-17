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


#include "accel_req.h"

#include "slot.h"
#include "hw_task.h"
#include "sw_task_client.h"

//---------------------------------------------------------------------------------------------

const struct phy_bit *accel_req_get_phy_bit(const struct accel_req *self)
{
	assert(self);
	assert(self->slot);
	assert(self->hw_task);

	return hw_task_get_bit_phy(self->hw_task, slot_get_index(self->slot));
}

int accel_req_notify_action(struct accel_req *self)
{
	assert(self);

	return sw_task_client_notify_action(self->sw_task_client);
}

