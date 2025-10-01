// main_tof.c
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include "uart_helpers.h"

#define TOF_ADC_CHANNEL 1 // A1

void adc_init()
{
  ADMUX = (1 << REFS0);
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
  uart_println("GP2Y0A21YK0F ToF bare-metal test (A1)");

  while (1)
  {
    uint16_t raw = adc_read(TOF_ADC_CHANNEL);
    float voltage = raw * (5.0f / 1023.0f);

    // Benaderingsformule (datasheet niet lineair) - af te stemmen met kalibratie
    // Voor kleine embedded builds powf() kan groot zijn; dit is eenvoudige benadering.
    float distance_cm;
    if (voltage <= 0.01f)
      distance_cm = 999.0f;
    else
    {
      // empirische benadering: distance = a * voltage^b
      // a en b hier gekozen als startpunt; kalibreer met metingen
      distance_cm = 27.86f * powf(voltage, -1.15f);
    }

    uart_print("ToF ADC: ");
    uart_print_u16(raw);
    uart_print("  Voltage: ");
    uart_print_f(voltage, 2);
    uart_print("V  Afstand (cm): ");
    uart_print_f(distance_cm, 2);
    uart_print("\r\n");

    _delay_ms(300);
  }
}
