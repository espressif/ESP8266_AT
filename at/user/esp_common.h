/*
 * File	: esp_common.h
 * This file is part of Espressif's AT+ command set program.
 * Copyright (C) 2013 - 2016, Espressif Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ESP_COMMON_H
#define __ESP_COMMON_H

#define ESP_PARAM_START_SEC   0x3C
#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2
#define ESP_PARAM_FLAG      3

struct esp_platform_sec_flag_param {
  uint8 flag;
  uint8 pad[3];
};

void user_esp_platform_load_param(void *param, uint16_t len);
void user_esp_platform_save_param(void *param, uint16_t len);

#endif
