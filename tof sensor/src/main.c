// main_tof_lookup.c
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "uart_helpers.h"

#define TOF_ADC_CHANNEL 1 // A1 = PC1

// ---------- ADC setup ----------
void adc_init()
{
  // Referentie = AVcc (5V), resultaat rechts gealigneerd
  ADMUX = (1 << REFS0);
  // ADC aan, prescaler 128 (16 MHz / 128 = 125 kHz)
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

uint16_t adc_read_avg(uint8_t ch, uint8_t samples)
{
  uint32_t sum = 0;
  for (uint8_t i = 0; i < samples; i++)
  {
    sum += adc_read(ch);
    _delay_ms(2);
  }
  return (uint16_t)(sum / samples);
}

// ---------- Lookup tabel uit datasheet (typische waarden) ----------
const float voltages[] = {3.0, 1.6, 1.1, 0.8, 0.65, 0.55, 0.45, 0.40};
const float distances[] = {10, 20, 30, 40, 50, 60, 70, 80};
const uint8_t N_POINTS = 8;

// Lineaire interpolatie tussen punten
float voltage_to_distance(float v)
{
  if (v > voltages[0])
    return distances[0]; // dichterbij dan 10 cm
  if (v < voltages[N_POINTS - 1])
    return 999.0; // verder dan 80 cm

  for (uint8_t i = 0; i < N_POINTS - 1; i++)
  {
    if (v <= voltages[i] && v >= voltages[i + 1])
    {
      float dv = voltages[i] - voltages[i + 1];
      float t = (v - voltages[i + 1]) / dv;
      return distances[i + 1] + t * (distances[i] - distances[i + 1]);
    }
  }
  return 999.0; // fallback
}

// ---------- Main ----------
int main(void)
{
  uart_init(9600);
  adc_init();
  uart_println("Sharp GP2Y0A21YK0F IR afstandssensor (lookup tabel)");

  while (1)
  {
    // Gemiddelde waarde voor minder ruis
    uint16_t raw = adc_read_avg(TOF_ADC_CHANNEL, 8);
    float voltage = raw * (5.0f / 1023.0f);
    float distance_cm = voltage_to_distance(voltage);

    uart_print("Raw: ");
    uart_print_u16(raw);
    uart_print("  U: ");
    uart_print_f(voltage, 2);
    uart_print(" V  Afstand: ");
    uart_print_f(distance_cm, 1);
    uart_println(" cm");

    _delay_ms(300);
  }
}
