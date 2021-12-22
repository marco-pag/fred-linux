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

#ifndef SYS_HW_CONFIG_H_
#define SYS_HW_CONFIG_H_

//---------------------------------------------------------------------------------------------

enum sys_slot_type {
    SYS_SLOT_NULL,
    SYS_SLOT_MASTER
};

enum sys_devcfg_type {
    SYS_DEVCFG_NULL,
    SYS_DEVCFG_FPGA_MGR
};

//---------------------------------------------------------------------------------------------

struct sys_hw_config {
    enum sys_slot_type slot_type;
    enum sys_devcfg_type devcfg_type;
};

//---------------------------------------------------------------------------------------------

static inline
enum sys_slot_type sys_hw_config_get_slot_type(const struct sys_hw_config *self)
{
    return self->slot_type;
}

static inline
void sys_hw_config_set_slot_type(struct sys_hw_config *self,
                                    enum sys_slot_type slot_type)
{
    self->slot_type = slot_type;
}

static inline
enum sys_devcfg_type sys_hw_config_get_devcfg_type(const struct sys_hw_config *self)
{
    return self->devcfg_type;
}

static inline
void sys_hw_config_set_devcfg_type(struct sys_hw_config *self,
                                    enum sys_devcfg_type devcfg_type)
{
    self->devcfg_type = devcfg_type;
}

//---------------------------------------------------------------------------------------------

#endif /* SYS_HW_CONFIG_H_ */
