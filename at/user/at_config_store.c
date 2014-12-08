/*
 * File	: at_config_store.c
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

#include <stdint.h>
#include <string.h>
#include "at_config_store.h"
#include "user_interface.h"
#include "esp_common.h"
#include "crc32.h"

static atConfig_t u_config;
static uint8_t u_config_loaded = 0;

atConfig_t ICACHE_FLASH_ATTR 
*atConfig_get(void)
{
	if(u_config_loaded != 1)
	{
		user_esp_platform_load_param(&u_config, sizeof(atConfig_t));
		u_config_loaded = 1;
	}
	return &u_config;
}

void ICACHE_FLASH_ATTR
atConfig_save(void)
{
	if(u_config.crc == crc32_checksum((uint8_t *)&u_config, (size_t)sizeof(atConfig_t) - 4))
	{
	  user_esp_platform_save_param(&u_config, sizeof(atConfig_t));
	}
	else 
	{
		//debug("config error"); 
	}
}

void ICACHE_FLASH_ATTR
atConfig_init_default(void)
{
	atConfig_t *config = atConfig_get();
	bzero(config, sizeof(atConfig_t));
	config->serial.baud_rate = 115200;
	config->crc = crc32_checksum((uint8_t *)config, (size_t)sizeof(atConfig_t) - 4);
	atConfig_save();
}

atConfig_t ICACHE_FLASH_ATTR 
*atConfig_init(void)
{
	atConfig_t *config = atConfig_get();
  if(config->crc != crc32_checksum((uint8_t *)config, (size_t)sizeof(atConfig_t) - 4))
  {
    atConfig_init_default();
  }
  return config;
}

/*
** test code
*/
//static atConfig_t test;
//
//void ICACHE_FLASH_ATTR
//user_date_test(void)
//{
//  bzero(&test, sizeof(atConfig_t));
//  
//  atConfig_t show;
//  char *p;
//  int i;
//
//  test.serial.baud_rate = 115200;
//  test.wifi_set.sta_setIp = 1;
//  test.wifi_set.ap_setIp = 2;
//  test.start.auto_trans = 1;
//  test.start.role_type = 1;
//  test.start.link_role.client.link_type = 1;
//  test.start.link_role.client.remote_target.remote_ip[3] = 168;
//  
//  test.crc = crc32_checksum((uint8_t *)&test, (size_t)(sizeof(atConfig_t) - 4));
//	
//  user_esp_platform_save_param(&test, sizeof(atConfig_t));
//
//  user_esp_platform_load_param(&show, sizeof(atConfig_t));
//
//  p = (char *)&show;
//  for(i; i<sizeof(atConfig_t); i++)
//  {
//    os_printf("0x%02X ", *p++);
//  }
//  os_printf("\r\nbaud:%d\r\n", show.serial.baud_rate);
//  os_printf("ip part:%d\r\n", show.start.link_role.client.remote_target.remote_ip[3]);
//  os_printf("crc32:0x%04X\r\n", show.crc);
//}
