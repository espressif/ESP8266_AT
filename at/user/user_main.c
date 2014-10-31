#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "at.h"

extern uint8_t at_wifiMode;

void user_init(void)
{
  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  at_wifiMode = wifi_get_opmode();
  os_printf("\r\nready!!!\r\n");
  uart0_sendStr("\r\nready\r\n");
  at_init();
}
