#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "at.h"

extern uint8_t at_wifiMode;
extern void user_esp_platform_load_param(void *param, uint16 len);

void user_init(void)
{
  uint8_t userbin;
  uint32_t upFlag;
  at_uartType tempUart;

  user_esp_platform_load_param((uint32 *)&tempUart, sizeof(at_uartType));
  if(tempUart.saved == 1)
  {
    uart_init(tempUart.baud, BIT_RATE_115200);
  }
  else
  {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
  }
  at_wifiMode = wifi_get_opmode();
  os_printf("\r\nready!!!\r\n");
  uart0_sendStr("\r\nready\r\n");
  at_init();
}
