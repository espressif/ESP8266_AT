/*
 * File	: at.h
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
#ifndef __AT_H
#define __AT_H

#include "c_types.h"

//#define at_busyTaskPrio        1
//#define at_busyTaskQueueLen    4

//#define ali

#define at_recvTaskPrio        0
#define at_recvTaskQueueLen    64

#define at_procTaskPrio        1
#define at_procTaskQueueLen    1

#define at_backOk        uart0_sendStr("\r\nOK\r\n")
#define at_backError     uart0_sendStr("\r\nERROR\r\n")
#define at_backTeError   "+CTE ERROR: %d\r\n"

typedef enum{
  at_statIdle,
  at_statRecving,
  at_statProcess,
  at_statIpSending,
  at_statIpSended,
  at_statIpTraning
}at_stateType;

typedef enum{
  m_init,
  m_wact,
  m_gotip,
  m_linked,
  m_unlink,
  m_wdact
}at_mdStateType;

typedef struct
{
	char *at_cmdName;
	int8_t at_cmdLen;
  void (*at_testCmd)(uint8_t id);
  void (*at_queryCmd)(uint8_t id);
  void (*at_setupCmd)(uint8_t id, char *pPara);
  void (*at_exeCmd)(uint8_t id);
}at_funcationType;

typedef struct
{
  uint32_t baud;
  uint32_t saved;
}at_uartType;

void at_init(void);
void at_cmdProcess(uint8_t *pAtRcvData);

#endif
