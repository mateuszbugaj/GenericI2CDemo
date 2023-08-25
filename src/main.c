/*
+--------+                              +--------+
| A168   |                  +---------+ | A168   |
| MASTER |                  |USB-UART | | SLAVE  |
|    PB0 +-> LED -> GND     |Converter| |    PB0 +-> LED -> GND
|    PD0 +------------------+ TX      | |        |
|    PD1 +------------------+ RX      | |        |
|    PB1 +-> BUTTON -> GND  +---------+ |        |
|    PD7 +--SCL(OUT)          SLC(OUT)--+ PD7    |
|    PD5 +--SCL(IN)            SCL(IN)--+ PD5    |
|    PD6 +--SDA(OUT)          SDA(OUT)--+ PD6    |
|    PB7 +--SDA(IN)            SDA(IN)--+ PB7    |
|    PB2 +-> GND (ROLE)                 |        |
+--------+                              +--------+ 
*/

#include <avr/io.h>
#include <avr/power.h>

#include "hal.h"
#include "usart.h"
#include "i2c.h"

#define MASTER_ADDR 51
#define SLAVE_ADDR 52

void write(uint8_t payload, uint8_t address);

HALPin sclOutPin = { .port = &PORTD, .pin = 7, .pullup = PULLUP_DISABLE };
HALPin sdaOutPin = { .port = &PORTD, .pin = 6, .pullup = PULLUP_DISABLE };
HALPin sclInPin = { .port = &PORTD, .pin = 5, .pullup = PULLUP_ENABLE };
HALPin sdaInPin = { .port = &PORTB, .pin = 7, .pullup = PULLUP_ENABLE };

I2C_Config i2c_config = {
    .respondToGeneralCall = true,
    .loggingLevel = 3};

int main(void) {
  clock_prescale_set(clock_div_1);
  usart_init();

  HALPin led = {&PORTB, 0};
  hal_pin_direction(led, OUTPUT);

  HALPin button = {&PORTB, 1, PULLUP_ENABLE};
  hal_pin_direction(button, INPUT);

  HALPin rolePin = {&PORTB, 2, PULLUP_ENABLE};
  hal_pin_direction(rolePin, INPUT);

  I2C_setPrintFunc(&usart_print);
  I2C_setPrintNumFunc(&usart_print_num);

  i2c_config.sclOutPin = sclOutPin;
  i2c_config.sdaOutPin = sdaOutPin;
  i2c_config.sclInPin = sclInPin;
  i2c_config.sdaInPin = sdaInPin;

  if(hal_pin_read(rolePin) == LOW){
    i2c_config.role = MASTER;
    i2c_config.addr = MASTER_ADDR;
  } else {
    i2c_config.role = SLAVE;
    i2c_config.addr = SLAVE_ADDR;
  }

  usart_print("Start...\r\n");
  I2C_init(&i2c_config);
  uint8_t counter = 0;
  while (1) {
    if(i2c_config.role == SLAVE){
      I2C_read();
    }

    if(i2c_config.role == MASTER){
      bool result = I2C_write(SLAVE_ADDR);
      if(result){
        while(1){
          result = I2C_write(counter++);
          if(result == false) return 0;
        }
      } else {
        I2C_logNum("Address not responding: ", SLAVE_ADDR, 1);
        return 0;
      }
    }
  }
}