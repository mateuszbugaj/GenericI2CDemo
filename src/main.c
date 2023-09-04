#define TEST_MODE 2

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

+----------+
| MPU-6000 |
|      SCL +--SCL
|      SDA +--SDA
+----------+

+--------+
| AS5600 |
|    SCL +--SCL
|    SDA +--SDA
+--------+

TEST_MODE: 0
Bi-directional communication of two microcontrollers. (Master - Slave)

MASTER                                  SLAVE
|                                           |
+-----------[Addres + WRITE]--------------->+
+-----------[Counter]---------------------->+
|                                           |
+-----------[Addres + READ]---------------->+
+<----------[Counter + 1]-------------------+
|                                           |

TEST_MODE: 1
Bi-directional communication with MPU-6000 gyroscope.

MASTER                               MPU-6000
|                                           |
+-----------[Address + WRITE]-------------->+
+-----------[WHO AM I Register]------------>+
|                                           |
+-----------[Adress + READ]---------------->+
+<----------[WHO AM I Register value]-------+
|                                           |

TEST_MODE: 2
Bi-directional continous reading of 12-bit angle from AS5600 encoder.

MASTER                                 AS5600
|                                           |
+-----------[Address + WRITE]-------------->+
+-----------[ANGLE L Register]------------->+
|                                           |
+-----------[Address + READ]--------------->+
+<----------[Angle L Register value]--------+
|                                           |
|                                           |
+-----------[Address + WRITE]-------------->+
+-----------[ANGLE H Register]------------->+
|                                           |
+-----------[Address + READ]--------------->+
+<----------[Angle H Register value]--------+
|                                           |

*/

#include <avr/io.h>
#include <avr/power.h>

#include "hal.h"
#include "usart.h"
#include "i2c.h"
#include "mpu6000.h"
#include "as5600.h"

#define MASTER_ADDR 51
#define SLAVE_ADDR 52

uint8_t readRegister(uint8_t reqisterAddress);

HALPin sclOutPin = { .port = &PORTD, .pin = 7, .pullup = PULLUP_DISABLE };
HALPin sdaOutPin = { .port = &PORTD, .pin = 6, .pullup = PULLUP_DISABLE };
HALPin sclInPin = { .port = &PORTD, .pin = 5, .pullup = PULLUP_ENABLE };
HALPin sdaInPin = { .port = &PORTB, .pin = 7, .pullup = PULLUP_ENABLE };

I2C_Config i2c_config = {
    .respondToGeneralCall = true,
    .timeUnit = 20, // 20 -> 2 bytes per second
    .loggingLevel = 1};

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
  uint8_t angle_H, angle_L;
  while (1) {
    if(i2c_config.role == SLAVE){
      I2C_read();
      if(I2C_newByteReceived(true)){
        uint8_t payload = I2C_lastByte();
        I2C_logNum("> Got new byte: ", payload, 1);
        I2C_write(++payload);
      }
    }

    if(i2c_config.role == MASTER){
#if TEST_MODE == 0
      I2C_sendStartCondition();
      if(I2C_writeAddress(SLAVE_ADDR, WRITE)){
        I2C_write(counter);
        I2C_sendRepeatedStartCondition();
        I2C_writeAddress(SLAVE_ADDR, READ);
        uint8_t response = I2C_receive(true);
        I2C_logNum("> Got response: ", response, 1);
        counter = response;
      } else {
        I2C_logNum("> Address not responding: ", SLAVE_ADDR, 1);
        return 0;
      }
      I2C_sendStopCondition();
#elif TEST_MODE == 1
      I2C_sendStartCondition();
      I2C_writeAddress(MPU6000_ADDR, WRITE);
      I2C_write(MPU6000_WHO_AM_I_REG);
      I2C_sendRepeatedStartCondition();
      I2C_writeAddress(MPU6000_ADDR, READ);
      uint8_t result = I2C_receive(false);
      I2C_sendStopCondition();

      I2C_logNum("Who am I", result, 1);
      I2C_logNum("Should be: ", MPU6000_ADDR, 1);
      return 0;
#elif TEST_MODE == 2
      angle_H = readRegister(AS5600_ANGLE_H);
      angle_L = readRegister(AS5600_ANGLE_L);
      uint16_t combined = (angle_H << 8) | angle_L;
      float translated = 360 * (combined / 4096.0);
      usart_print_float(translated, 2);
      usart_print("\n\r");

#endif
    }
  }
}

uint8_t readRegister(uint8_t reqisterAddress){
  uint8_t value;

  I2C_sendStartCondition();
  if(I2C_writeAddress(AS5600_ADDR, WRITE)){
    I2C_write(reqisterAddress);
    I2C_sendRepeatedStartCondition();
    I2C_writeAddress(AS5600_ADDR, READ);
    value = I2C_receive(false);
  } else {
    I2C_logNum("> Address not responding: ", AS5600_ADDR, 1);
    return 0;
  }
  I2C_sendStopCondition();

  return value;
}
