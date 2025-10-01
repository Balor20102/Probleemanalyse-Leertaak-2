// main_ldr.c
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "uart_helpers.h"

#define LDR_ADC_CHANNEL 0 // A0

void adc_init()
{
  // Referentie = AVcc (5V), ADC0 geselecteerd later
  ADMUX = (1 << REFS0);
  // ADC Enable, prescaler 128 => 16MHz/128 = 125kHz
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
  ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC))
    ;
  return ADC;
}

int main(void)
{
  uart_init(9600);
  adc_init();

  uart_println("LDR bare-metal test (ADC A0)");

  while (1)
  {
    uint16_t v = adc_read(LDR_ADC_CHANNEL);
    uart_print("LDR ADC: ");
    uart_print_u16(v);
    uart_print("\r\n");
    _delay_ms(200);
  }
}
