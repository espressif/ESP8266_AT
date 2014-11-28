/*
 * File	: at_wifiCmd.c
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
#include "user_interface.h"
#include "at.h"
#include "at_wifiCmd.h"
#include "osapi.h"
#include "c_types.h"
#include "mem.h"

at_mdStateType mdState = m_unlink;

extern BOOL specialAtState;
extern at_stateType at_state;
extern at_funcationType at_fun[];

uint8_t at_wifiMode;
os_timer_t at_japDelayChack;

/** @defgroup AT_WSIFICMD_Functions
  * @{
  */

/**
  * @brief  Copy param from receive data to dest.
  * @param  pDest: point to dest
  * @param  pSrc: point to source
  * @param  maxLen: copy max number of byte
  * @retval the length of param
  *   @arg -1: failure
  */
int8_t ICACHE_FLASH_ATTR
at_dataStrCpy(void *pDest, const void *pSrc, int8_t maxLen)
{
//  assert(pDest!=NULL && pSrc!=NULL);

  char *pTempD = pDest;
  const char *pTempS = pSrc;
  int8_t len;

  if(*pTempS != '\"')
  {
    return -1;
  }
  pTempS++;
  for(len=0; len<maxLen; len++)
  {
    if(*pTempS == '\"')
    {
      *pTempD = '\0';
      break;
    }
    else
    {
      *pTempD++ = *pTempS++;
    }
  }
  if(len == maxLen)
  {
    return -1;
  }
  return len;
}

/**
  * @brief  Test commad of set wifi mode.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_testCmdCwmode(uint8_t id)
{
  char temp[32];
  os_sprintf(temp, "%s:(1-3)\r\n", at_fun[id].at_cmdName);
  uart0_sendStr(temp);
  at_backOk;
}

/**
  * @brief  Query commad of set wifi mode.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_queryCmdCwmode(uint8_t id)
{
  char temp[32];

  at_wifiMode = wifi_get_opmode();
  os_sprintf(temp, "%s:%d\r\n", at_fun[id].at_cmdName, at_wifiMode);
  uart0_sendStr(temp);
  at_backOk;
}

/**
  * @brief  Setup commad of set wifi mode.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCwmode(uint8_t id, char *pPara)
{
  uint8_t mode;
  char temp[32];

  pPara++;
  mode = atoi(pPara);
  if(mode == at_wifiMode)
  {
//    uart0_sendStr("no change\r\n");
    at_backOk;
    return;
  }
  if((mode >= 1) && (mode <= 3))
  {
    ETS_UART_INTR_DISABLE();
    wifi_set_opmode(mode);
    ETS_UART_INTR_ENABLE();
    at_wifiMode = mode;
    at_backOk;
//    system_restart();
  }
  else
  {
    at_backError;
  }
}

/**
  * @brief  Wifi ap scan over callback to display.
  * @param  arg: contain the aps information
  * @param  status: scan over status
  * @retval None
  */
static void ICACHE_FLASH_ATTR
scan_done(void *arg, STATUS status)
{
  uint8 ssid[33];
  char temp[128];

  if (status == OK)
  {
    struct bss_info *bss_link = (struct bss_info *)arg;
    bss_link = bss_link->next.stqe_next;//ignore first

    while (bss_link != NULL)
    {
      os_memset(ssid, 0, 33);
      if (os_strlen(bss_link->ssid) <= 32)
      {
        os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
      }
      else
      {
        os_memcpy(ssid, bss_link->ssid, 32);
      }
      os_sprintf(temp,"+CWLAP:(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
                 bss_link->authmode, ssid, bss_link->rssi,
                 MAC2STR(bss_link->bssid),bss_link->channel);
      uart0_sendStr(temp);
      bss_link = bss_link->next.stqe_next;
    }
    at_backOk;
  }
  else
  {
//  	os_sprintf(temp,"err, scan status %d\r\n", status);
//  	uart0_sendStr(temp);
    at_backError;
  }
  specialAtState = TRUE;
  at_state = at_statIdle;
}

//void ICACHE_FLASH_ATTR
//at_testCmdCwjap(uint8_t id)
//{
//  wifi_station_scan(scan_done);
//  specialAtState = FALSE;
//}

void ICACHE_FLASH_ATTR
at_setupCmdCwlap(uint8_t id, char *pPara)
{
  struct scan_config config;
  int8_t len;
  char ssid[32];
  char bssidT[18];
  char bssid[6];
  uint8_t channel;
  uint8_t i;

  pPara++;
//  os_printf("%x\r\n",*pPara);////
//  os_bzero(ssid, 32);

  len = at_dataStrCpy(ssid, pPara, 32);
//  os_printf("%d\r\n",len);////

  if(len == -1)
  {
    at_backError;
//    uart0_sendStr(at_backTeError"11\r\n");
    return;
  }
  if(len == 0)
  {
    config.ssid = NULL;
  }
  else
  {
    config.ssid = ssid;
  }
  pPara += (len+3);
//  os_printf("%x\r\n",*pPara);/////
  len = at_dataStrCpy(bssidT, pPara, 18);
//  os_printf("%d\r\n",len);////
  if(len == -1)
  {
    at_backError;
  //    uart0_sendStr(at_backTeError"11\r\n");
    return;
  }
  if(len == 0)
  {
    config.bssid = NULL;
  }
  else
  {
    if(os_str2macaddr(bssid, bssidT) == 0)
    {
      at_backError;
      return;
    }
    config.bssid = bssid;
  }
//  if(*pPara == ',')
//  {
//    config.bssid = NULL;
//  }
//  else
//  {
//    os_memcpy(bssidT,pPara,17);
//    bssidT[17] = '\0';
//    os_str2macaddr(bssid, bssidT);
//    config.bssid = bssid;
//  }
  pPara += (len+3);
  config.channel = atoi(pPara);

//  os_printf("%s,%s,%d\r\n",
//            config.ssid,
//            config.bssid,
//            config.channel);
  if(wifi_station_scan(&config, scan_done))
  {
    specialAtState = FALSE;
  }
  else
  {
    at_backError;
  }
}

/**
  * @brief  Execution commad of list wifi aps.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdCwlap(uint8_t id)//need add mode chack
{
  if(at_wifiMode == SOFTAP_MODE)
  {
    at_backError;
    return;
  }
  wifi_station_scan(NULL,scan_done);
  specialAtState = FALSE;
}

/**
  * @brief  Query commad of join to wifi ap.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_queryCmdCwjap(uint8_t id)
{
  char temp[64];
  struct station_config stationConf;
  wifi_station_get_config(&stationConf);
  struct ip_info pTempIp;

  wifi_get_ip_info(0x00, &pTempIp);
  if(pTempIp.ip.addr == 0)
  {
    uart0_sendStr("No AP\r\n");
    at_backError;
    //    os_sprintf(temp, at_backTeError, 4);
    //    uart0_sendStr(at_backTeError"4\r\n");
    return;
  }
  mdState = m_gotip;
  if(stationConf.ssid != 0)
  {
    os_sprintf(temp, "%s:\"%s\"\r\n", at_fun[id].at_cmdName, stationConf.ssid);
    uart0_sendStr(temp);
    at_backOk;
  }
  else
  {
    at_backError;
    //    os_sprintf(temp, at_backTeError, 5);
    //    uart0_sendStr(at_backTeError"5\r\n");
  }
}

/**
  * @brief  Transparent data through ip.
  * @param  arg: no used
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_japChack(void *arg)
{
  static uint8_t chackTime = 0;
  uint8_t japState;
  char temp[32];

  os_timer_disarm(&at_japDelayChack);
  chackTime++;
  japState = wifi_station_get_connect_status();
  if(japState == STATION_GOT_IP)
  {
    chackTime = 0;
    at_backOk;
    specialAtState = TRUE;
    at_state = at_statIdle;
    return;
  }
  else if(chackTime >= 7)
  {
    wifi_station_disconnect();
    chackTime = 0;
    os_sprintf(temp,"+CWJAP:%d\r\n",japState);
    uart0_sendStr(temp);
    uart0_sendStr("\r\nFAIL\r\n");
    specialAtState = TRUE;
    at_state = at_statIdle;
    return;
  }
  os_timer_arm(&at_japDelayChack, 2000, 0);

}

/**
  * @brief  Setup commad of join to wifi ap.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCwjap(uint8_t id, char *pPara)
{
  char temp[64];
  struct station_config stationConf;

  int8_t len;

//  stationConf /////////
  os_bzero(&stationConf, sizeof(struct station_config));
  if (at_wifiMode == SOFTAP_MODE)
  {
    at_backError;
    return;
  }
  pPara++;
  len = at_dataStrCpy(&stationConf.ssid, pPara, 32);
  if(len != -1)
  {
    pPara += (len+3);
    len = at_dataStrCpy(&stationConf.password, pPara, 64);
  }
  if(len != -1)
  {
    wifi_station_disconnect();
    mdState = m_wdact;
    ETS_UART_INTR_DISABLE();
    wifi_station_set_config(&stationConf);
    ETS_UART_INTR_ENABLE();
    wifi_station_connect();
//    if(1)
//    {
//      mdState = m_wact;
//    }
//    os_sprintf(temp,"%s:%s,%s\r\n",
//               at_fun[id].at_cmdName,
//               stationConf.ssid,
//               stationConf.password);
//    uart0_sendStr(temp);
    os_timer_disarm(&at_japDelayChack);
    os_timer_setfn(&at_japDelayChack, (os_timer_func_t *)at_japChack, NULL);
    os_timer_arm(&at_japDelayChack, 3000, 0);
    specialAtState = FALSE;
  }
  else
  {
    at_backError;
  }
}

/**
  * @brief  Test commad of quit wifi ap.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_testCmdCwqap(uint8_t id)
{
  at_backOk;
}

/**
  * @brief  Execution commad of quit wifi ap.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdCwqap(uint8_t id)
{
  wifi_station_disconnect();
  mdState = m_wdact;
  at_backOk;
}

/**
  * @brief  Query commad of module as wifi ap.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_queryCmdCwsap(uint8_t id)
{
  struct softap_config apConfig;
  char temp[128];

  if(at_wifiMode == STATION_MODE)
  {
    at_backError;
    return;
  }
  wifi_softap_get_config(&apConfig);
  os_sprintf(temp,"%s:\"%s\",\"%s\",%d,%d\r\n",
             at_fun[id].at_cmdName,
             apConfig.ssid,
             apConfig.password,
             apConfig.channel,
             apConfig.authmode);
  uart0_sendStr(temp);
  at_backOk;
}

/**
  * @brief  Setup commad of module as wifi ap.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCwsap(uint8_t id, char *pPara)
{
  char temp[64];
  int8_t len,passLen;
  struct softap_config apConfig;

  os_bzero(&apConfig, sizeof(struct softap_config));
  wifi_softap_get_config(&apConfig);

  if(at_wifiMode == STATION_MODE)
  {
    at_backError;
//    uart0_sendStr(at_backTeError"6\r\n");
    return;
  }
  pPara++;
  len = at_dataStrCpy(apConfig.ssid, pPara, 32);
//  os_printf("%x\r\n",*pPara);/////
//  os_printf("%s\r\n",apConfig.ssid);/////
  if(len < 1)
  {
//    uart0_sendStr("ssid ERROR\r\n");
    at_backError;
//    uart0_sendStr(at_backTeError"7\r\n");
    return;
  }
  pPara += (len+3);
  passLen = at_dataStrCpy(apConfig.password, pPara, 64);
//  os_printf("%x\r\n",*pPara);/////
//  os_printf("%s\r\n",apConfig.password);/////
  if(passLen == -1 )
  {
//    uart0_sendStr("pwd ERROR\r\n");
    at_backError;
//    uart0_sendStr(at_backTeError"8\r\n");
    return;
  }
  pPara += (passLen+3);
//  os_printf("%x\r\n",*pPara);/////
  apConfig.channel = atoi(pPara);
  if(apConfig.channel<1 || apConfig.channel>13)
  {
//    uart0_sendStr("ch ERROR\r\n");
    at_backError;
//    uart0_sendStr(at_backTeError"9\r\n");
    return;
  }
  pPara++;
  pPara = strchr(pPara, ',');
  pPara++;
//  os_printf("%x\r\n",*pPara);/////
  apConfig.authmode = atoi(pPara);
  if(apConfig.authmode >= 5)
  {
//    uart0_sendStr("s ERROR\r\n");
    at_backError;
//    uart0_sendStr(at_backTeError"10\r\n");
    return;
  }
  if((apConfig.authmode != 0)&&(passLen < 5))
  {
    at_backError;
//    uart0_sendStr(at_backTeError"8\r\n");
    return;
  }
//  os_sprintf(temp,"%s,%s,%d,%d\r\n",
//             apConfig.ssid,
//             apConfig.password,
//             apConfig.channel,
//             apConfig.authmode);
//  uart0_sendStr(temp);
  ETS_UART_INTR_DISABLE();
  wifi_softap_set_config(&apConfig);
  ETS_UART_INTR_ENABLE();
  at_backOk;
//  system_restart();
}

void ICACHE_FLASH_ATTR
at_exeCmdCwlif(uint8_t id)
{
  struct station_info *station;
  struct station_info *next_station;
  char temp[128];

  if(at_wifiMode == STATION_MODE)
  {
    at_backError;
    return;
  }
  station = wifi_softap_get_station_info();
  while(station)
  {
    os_sprintf(temp, "%d.%d.%d.%d,"MACSTR"\r\n",
               IP2STR(&station->ip), MAC2STR(station->bssid));
    uart0_sendStr(temp);
    next_station = STAILQ_NEXT(station, next);
    os_free(station);
    station = next_station;
  }
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_queryCmdCwdhcp(uint8_t id)
{
	char temp[32];
	
}

void ICACHE_FLASH_ATTR
at_setupCmdCwdhcp(uint8_t id, char *pPara)
{
	uint8_t mode,opt;
	int8_t ret = 0;

	pPara++;
	mode = 0;
	mode = atoi(pPara);
	pPara ++;
	pPara = strchr(pPara, ',');
	pPara++;
	opt = atoi(pPara);
	
	switch (mode)
	{
	case 0:
	  if(opt)
	  {
	  	ret = wifi_softap_dhcps_start();
	  }
	  else 
	  {
	  	ret = wifi_softap_dhcps_stop();
	  }
		break;
	
	case 1:
		if(opt)
	  {
	  	ret = wifi_station_dhcpc_start();
	  }
	  else 
	  {
	  	ret = wifi_station_dhcpc_stop();
	  }
		break;
	
	case 2:
		if(opt)
	  {
	  	ret = wifi_softap_dhcps_start();
	  	ret |= wifi_station_dhcpc_start();
	  }
	  else 
	  {
	  	ret = wifi_softap_dhcps_stop();
	  	ret |= wifi_station_dhcpc_stop();
	  }
		break;	
	
	default:
			
		break;		
	}
	if(ret)
	{
	  at_backOk;
	}
	else 
	{
		at_backError;
	}
}

void ICACHE_FLASH_ATTR
at_queryCmdCipstamac(uint8_t id)
{
	char temp[64];
  uint8 bssid[6];
  
  os_sprintf(temp, "%s:", at_fun[id].at_cmdName);
  uart0_sendStr(temp);

  wifi_get_macaddr(STATION_IF, bssid);
  os_sprintf(temp, "\""MACSTR"\"\r\n", MAC2STR(bssid));
  uart0_sendStr(temp);
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCipstamac(uint8_t id, char *pPara)
{
	int8_t len,i;
  uint8 bssid[6];
  char temp[64];
  
	pPara++;
	
  len = at_dataStrCpy(temp, pPara, 32);
  if(len != 17)
  {
    at_backError;
    return;
  }

  pPara++; 
 
  for(i=0;i<6;i++)
  {
    bssid[i] = strtol(pPara,&pPara,16);
    pPara += 1;
  }
 
  os_printf(MACSTR"\r\n", MAC2STR(bssid));
  wifi_set_macaddr(STATION_IF, bssid);
	at_backOk;
}


void ICACHE_FLASH_ATTR
at_queryCmdCipapmac(uint8_t id)
{
	char temp[64];
  uint8 bssid[6];
  
  os_sprintf(temp, "%s:", at_fun[id].at_cmdName);
  uart0_sendStr(temp);

  wifi_get_macaddr(SOFTAP_IF, bssid);
  os_sprintf(temp, "\""MACSTR"\"\r\n", MAC2STR(bssid));
  uart0_sendStr(temp);
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCipapmac(uint8_t id, char *pPara)
{
  int8_t len,i;
  uint8 bssid[6];
  char temp[64];
  
	pPara++;
	
  len = at_dataStrCpy(temp, pPara, 32);
  if(len != 17)
  {
    at_backError;
    return;
  }

  pPara++; 
 
  for(i=0;i<6;i++)
  {
    bssid[i] = strtol(pPara,&pPara,16);
    pPara += 1;
  }
 
  os_printf(MACSTR"\r\n", MAC2STR(bssid));
  wifi_set_macaddr(SOFTAP_IF, bssid);
	at_backOk;
}

void ICACHE_FLASH_ATTR
at_queryCmdCipsta(uint8_t id)
{
	struct ip_info pTempIp;
  char temp[64];
  
  wifi_get_ip_info(0x00, &pTempIp);
  os_sprintf(temp, "%s:", at_fun[id].at_cmdName);
  uart0_sendStr(temp);

  os_sprintf(temp, "\"%d.%d.%d.%d\"\r\n", IP2STR(&pTempIp.ip));
  uart0_sendStr(temp);
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCipsta(uint8_t id, char *pPara)
{
	struct ip_info pTempIp;
  int8_t len;
  char temp[64];
  
  wifi_station_dhcpc_stop();
  
  pPara++;
	
  len = at_dataStrCpy(temp, pPara, 32);
  if(len == -1)
  {
    at_backError;
    return;
  }
  pPara++;
  wifi_get_ip_info(0x00, &pTempIp);
  pTempIp.ip.addr = ipaddr_addr(temp);

  os_printf("%d.%d.%d.%d\r\n",
                 IP2STR(&pTempIp.ip));

  if(!wifi_set_ip_info(0x00, &pTempIp))
  {
    at_backError;
    wifi_station_dhcpc_start();
    return;
  }
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_queryCmdCipap(uint8_t id)
{
	struct ip_info pTempIp;
  char temp[64];
  
  wifi_get_ip_info(0x01, &pTempIp);
  os_sprintf(temp, "%s:", at_fun[id].at_cmdName);
  uart0_sendStr(temp);

  os_sprintf(temp, "\"%d.%d.%d.%d\"\r\n", IP2STR(&pTempIp.ip));
  uart0_sendStr(temp);
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCipap(uint8_t id, char *pPara)
{
	struct ip_info pTempIp;
  int8_t len;
  char temp[64];
  
  wifi_softap_dhcps_stop();
  
  pPara++;
	
  len = at_dataStrCpy(temp, pPara, 32);
  if(len == -1)
  {
    at_backError;
    return;
  }
  pPara++;
  wifi_get_ip_info(0x01, &pTempIp);
  pTempIp.ip.addr = ipaddr_addr(temp);

  os_printf("%d.%d.%d.%d\r\n",
                 IP2STR(&pTempIp.ip));

  if(!wifi_set_ip_info(0x01, &pTempIp))
  {
    at_backError;
    wifi_softap_dhcps_start();
    return;
  }
  wifi_softap_dhcps_start();
  at_backOk;
}

/**
  * @}
  */
