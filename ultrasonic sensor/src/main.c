// main_hcsr04_fixed.c
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "uart_helpers.h"

// PB1 = Arduino D9 (TRIG)
// PB2 = Arduino D10 (ECHO)
#define TRIG_PIN PB1
#define ECHO_PIN PB2

// Timeout counts (prevent blokkering)
#define TIMEOUT_WAIT_HIGH 30000UL
#define TIMEOUT_WAIT_LOW 60000UL

void pins_init()
{
  // TRIG = output on PORTB
  DDRB |= (1 << TRIG_PIN);
  // ECHO = input on PORTB
  DDRB &= ~(1 << ECHO_PIN);
  // Zorg dat TRIG low is
  PORTB &= ~(1 << TRIG_PIN);
  // Zorg dat pull-up voor ECHO UIT is (we willen geen interne pull-up)
  PORTB &= ~(1 << ECHO_PIN);
}

uint16_t measure_ticks_timeout()
{
  uint32_t tcount = 0;

  // zorg dat trig LOW
  PORTB &= ~(1 << TRIG_PIN);
  _delay_us(2);

  // Trigger 10us pul s
  PORTB |= (1 << TRIG_PIN);
  _delay_us(10);
  PORTB &= ~(1 << TRIG_PIN);

  // Wacht op ECHO = HIGH (met timeout)
  uint32_t to = TIMEOUT_WAIT_HIGH;
  while (!(PINB & (1 << ECHO_PIN)))
  {
    if (--to == 0)
      return 0xFFFF; // timeout indicator
  }

  // Start timer1 (prescaler 8)
  TCNT1 = 0;
  TCCR1A = 0;
  TCCR1B = (1 << CS11); // prescaler 8

  // Wacht tot ECHO = LOW (met timeout)
  to = TIMEOUT_WAIT_LOW;
  while (PINB & (1 << ECHO_PIN))
  {
    if (--to == 0)
    {
      TCCR1B = 0;
      return 0xFFFF;
    }
  }

  // stop timer
  TCCR1B = 0;
  uint16_t ticks = TCNT1;
  return ticks;
}

int main(void)
{
  uart_init(9600); // UART gebruikt PD0/PD1 — geen conflict meer
  pins_init();

  uart_println("HC-SR04 bare-metal test - fixed pins");

  while (1)
  {
    uint16_t t = measure_ticks_timeout();
    if (t == 0xFFFF)
    {
      uart_println("Timeout");
    }
    else
    {
      // prescaler 8 => tick = 0.5 us
      // afstand cm = (time_us / 58). time_us = ticks * 0.5
      float distance_cm = ((float)t * 0.5f) / 58.0f;
      uart_print("Afstand (cm): ");
      uart_print_f(distance_cm, 2);
      uart_print("\r\n");
    }
    _delay_ms(200);
  }
}
