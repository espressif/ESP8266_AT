/*
 * File	: at_wifiCmd.h
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
#ifndef __AT_WIFICMD_H
#define __AT_WIFICMD_H

void at_testCmdCwmode(uint8_t id);
void at_queryCmdCwmode(uint8_t id);
void at_setupCmdCwmode(uint8_t id, char *pPara);

//void at_testCmdCwjap(uint8_t id);
void at_queryCmdCwjap(uint8_t id);
void at_setupCmdCwjap(uint8_t id, char *pPara);

void at_setupCmdCwlap(uint8_t id, char *pPara);
void at_exeCmdCwlap(uint8_t id);

void at_testCmdCwqap(uint8_t id);
void at_exeCmdCwqap(uint8_t id);

void at_queryCmdCwsap(uint8_t id);
void at_setupCmdCwsap(uint8_t id, char *pPara);

void at_exeCmdCwlif(uint8_t id);

void at_queryCmdCwdhcp(uint8_t id);
void at_setupCmdCwdhcp(uint8_t id, char *pPara);

void at_queryCmdCipstamac(uint8_t id);
void at_setupCmdCipstamac(uint8_t id, char *pPara);

void at_queryCmdCipapmac(uint8_t id);
void at_setupCmdCipapmac(uint8_t id, char *pPara);

void at_queryCmdCipsta(uint8_t id);
void at_setupCmdCipsta(uint8_t id, char *pPara);

void at_queryCmdCipap(uint8_t id);
void at_setupCmdCipap(uint8_t id, char *pPara);

#endif
