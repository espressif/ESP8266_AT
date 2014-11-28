/*
 * File	: at_baseCmd.c
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
#include <stdlib.h>
#include "osapi.h"
#include "c_types.h"
#include "at.h"
#include "at_baseCmd.h"
#include "user_interface.h"
#include "at_version.h"
#include "version.h"
#include "driver/uart_register.h"

/** @defgroup AT_BASECMD_Functions
  * @{
  */

extern BOOL echoFlag;

typedef struct
{
    char flag;
    char reserve[3];
}updateFlagType;
  
/**
  * @brief  Execution commad of AT.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdNull(uint8_t id)
{
  at_backOk;
}

/**
  * @brief  Enable or disable Echo.
  * @param  id: command id number
  * @param  pPara:
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdE(uint8_t id, char *pPara)
{
//  os_printf("%c\n",*pPara);
  if(*pPara == '0')
  {
    echoFlag = FALSE;
  }
  else if(*pPara == '1')
  {
    echoFlag = TRUE;
  }
  else
  {
    at_backError;
    return;
  }
  at_backOk;
}

/**
  * @brief  Execution commad of restart.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdRst(uint8_t id)
{
  at_backOk;
  system_restart();
}

/**
  * @brief  Execution commad of version.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdGmr(uint8_t id)
{
  char temp[64];

  os_sprintf(temp, AT_VERSION);
  uart0_sendStr(temp);
  os_sprintf(temp,"%06X\r\n", SDK_VERSION);
  uart0_sendStr(temp);
  at_backOk;
}

#ifdef ali
//#define upFlagSector  60
/**
  * @brief  Through uart to update
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdUpdate(uint8_t id)
{
  char temp[32];
  updateFlagType upFlag;

  os_sprintf(temp,"Is about to restart\r\n");
  uart0_sendStr(temp);

  spi_flash_read(60 * 4096, (uint32 *)&upFlag, sizeof(updateFlagType));
//  os_printf("%X\r\n",upFlag.flag);
//  os_printf("%X\r\n",upFlag.reserve[0]);
//  os_printf("%X\r\n",upFlag.reserve[1]);
//  os_printf("%X\r\n",upFlag.reserve[2]);
  upFlag.flag = 1;
  spi_flash_erase_sector(60);
  spi_flash_write(60 * 4096, (uint32 *)&upFlag, sizeof(updateFlagType));
//  spi_flash_read(60 * 4096, (uint32 *)&upFlag, sizeof(updateFlagType));
//  os_printf("%X\r\n",upFlag.flag);
//  os_printf("%X\r\n",upFlag.reserve[0]);
//  os_printf("%X\r\n",upFlag.reserve[1]);
//  os_printf("%X\r\n",upFlag.reserve[2]);
  os_delay_us(10000);
  system_reboot_from(0x00);
}
#endif

#ifdef ali
void ICACHE_FLASH_ATTR
at_setupCmdMpinfo(uint8_t id, char *pPara)
{
  uint32 t;
  char temp[32];

  pPara++;
  t = strtol(pPara,NULL,16);
  os_sprintf(temp,"1st:%x\r\n",t);
  uart0_sendStr(temp);

  pPara = strchr(pPara, ',');

  pPara++;
  t = strtol(pPara,NULL,16);
  os_sprintf(temp,"2nd:%x\r\n",t);
  uart0_sendStr(temp);
}
#endif


#define ESP_PARAM_START_SEC   0x3C

#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2
#define ESP_PARAM_FLAG      3
struct esp_platform_sec_flag_param {
    uint8 flag;
    uint8 pad[3];
};

/******************************************************************************
 * FunctionName : user_esp_platform_load_param
 * Description  : load parameter from flash, toggle use two sector by flag value.
 * Parameters   : param--the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_load_param(void *param, uint16 len)
{
    struct esp_platform_sec_flag_param flag;

    spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                   (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));

    if (flag.flag == 0) {
        spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
                       (uint32 *)param, len);
    } else {
        spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
                       (uint32 *)param, len);
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_save_param
 * Description  : toggle save param to two sector by flag value,
 *              : protect write and erase data while power off.
 * Parameters   : param -- the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_save_param(void *param, uint16 len)
{
    struct esp_platform_sec_flag_param flag;

    spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                       (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));

    if (flag.flag == 0) {
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)param, len);
        flag.flag = 1;
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_FLAG);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));
    } else {
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)param, len);
        flag.flag = 0;
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_FLAG);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));
    }
}


void ICACHE_FLASH_ATTR
at_setupCmdIpr(uint8_t id, char *pPara)
{
  at_uartType tempUart;

  pPara++;
  tempUart.baud = atoi(pPara);
  if((tempUart.baud>(UART_CLK_FREQ / 16))||(tempUart.baud == 0))
  {
    at_backError;
    return;
  }
  while(TRUE)
  {
    uint32_t fifo_cnt = READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT<<UART_TXFIFO_CNT_S);
    if((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) == 0)
    {
      break;
    }
  }
  os_delay_us(10000);
  uart_div_modify(0, UART_CLK_FREQ / tempUart.baud);
  tempUart.saved = 1;
  user_esp_platform_save_param((uint32 *)&tempUart, sizeof(at_uartType));
//  spi_flash_read(60 * 4096, (uint32 *)&upFlag, sizeof(updateFlagType));
//  //  os_printf("%X\r\n",upFlag.flag);
//  //  os_printf("%X\r\n",upFlag.reserve[0]);
//  //  os_printf("%X\r\n",upFlag.reserve[1]);
//  //  os_printf("%X\r\n",upFlag.reserve[2]);
//    upFlag.flag = 1;
//    spi_flash_erase_sector(60);
//    spi_flash_write(60 * 4096, (uint32 *)&upFlag, sizeof(updateFlagType));
//  //  spi_flash_read(60 * 4096, (uint32 *)&upFlag, sizeof(updateFlagType));
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdGslp(uint8_t id, char *pPara)
{
	uint32_t n;
	pPara++;
	
	n = atoi(pPara);
	at_backOk;
	system_deep_sleep(n*1000);
}
/**
  * @}
  */

