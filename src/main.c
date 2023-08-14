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
|    PD7 +-> SCL
|    PD6 +-> SDA
+--------+
*/

/* 8-bit Timer/Counter0 */
float timerInterval = 100;
volatile int timerOverflowCount = 0;
void setupTimer() {
    TCCR0A = (1 << WGM01);               // CTC Mode, OCRA - TOP
    TCCR0B = (1 << CS02) | (1 << CS00);  // Prescaler to 1024
    OCR0A = 8;                           // with prescaler 1024 and 8Mhz CPU it gives 1,024ms
    TIMSK0 = (1 << OCIE0A);              // Output compare match A interrupt enable
    TIFR0 = 0;                           // clear flags

    sei();
}

/*
To compensate for hardware timer discrepancies, the timerCount value is multiplied by a correction factor.
With the current settings, the actual interval is 1.024ms instead of the desired 1ms.
*/
void setTimer(uint16_t ms){
  timerInterval = ms * 0.98;
}

enum State {
  SEND_START,
  SEND_STOP
};

enum State state;
HALPin SCLPin = { .port = &PORTD, .pin = 7, .pullup = PULLUP_DISABLE };
HALPin SDAPin = { .port = &PORTD, .pin = 6, .pullup = PULLUP_DISABLE };
int main(void) {
  clock_prescale_set(clock_div_1);
  setupTimer();

  usart_init();

  HALPin led = {&PORTB, 0};
  hal_pin_direction(led, OUTPUT);

  HALPin button = {&PORTB, 1, PULLUP_ENABLE};
  hal_pin_direction(button, INPUT);

  HALPin rolePin = {&PORTB, 2, PULLUP_ENABLE};
  hal_pin_direction(rolePin, INPUT);

  I2C_setPrintFunc(&usart_print);
  I2C_setPrintNumFunc(&usart_print_num);
  I2C_Config i2c_config = {
    .respondToGeneralCall = true,
    .loggingLevel = 4,
    .SCLPin = SCLPin,
    .SDAPin = SDAPin
    };

  if(hal_pin_read(rolePin) == HIGH){
    i2c_config.role = MASTER;
    i2c_config.addr = 51;
  } else {
    _delay_ms(500);
    i2c_config.role = SLAVE;
    i2c_config.addr = 52;
  }

  usart_print("Start...\r\n");
  I2C_init(&i2c_config);

  setTimer(1000);
  state = SEND_START;
  while (1) {}
}

ISR(TIMER0_COMPA_vect) {
  timerOverflowCount++;
  if (timerOverflowCount >= timerInterval){
    if(state == SEND_START){
      I2C_sendStartCondition();
      state = SEND_STOP;
      timerOverflowCount -= 500; // wait 500ms
      return;
    }

    if(state == SEND_STOP){
      I2C_sendStopCondition();
      state = SEND_START;
      timerOverflowCount -= 500; // wait 500ms
      return;
    }

    timerOverflowCount = 0;
  }
}