#include <avr/io.h>
#include <avr/power.h>
#include <avr/delay.h>

#include "hal.h"
#include "usart.h"
#include "i2c.h"

/*
+--------+
|  A168  |                  +--------+
|        |                  | USBasp |
|    PB0 +-> LED -> GND     |        |
|    PD0 +------------------+ TX     |
|    PD1 +------------------+ RX     |
|    PB1 +-> BUTTON -> GND  +--------+
+--------+

*/

int count = 0;
int main(void) {
  clock_prescale_set(clock_div_1);

  usart_init();

  HALPin led = {&PORTB, 0};
  hal_pin_direction(led, OUTPUT);

  HALPin button = {&PORTB, 1, PULLUP_ENABLE};
  hal_pin_direction(button, INPUT);

  I2C_setPrintFunc(&usart_print);
  I2C_Config i2c_config = {
    .addr = 123,
    .respondToGeneralCall = true,
    .loggingLevel = 4};  

  usart_print("Start...\r\n");
  I2C_init(&i2c_config);
  while (1) {
    if(hal_pin_read(button) == LOW){
      hal_pin_write(led, HIGH); 
      usart_print_num(count++);
      usart_print("\n\r");
    } else {
      hal_pin_write(led, LOW); 
    }
  }

}