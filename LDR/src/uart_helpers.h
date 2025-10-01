// uart_helpers.h  (of plak bovenin elk bestand)

#ifndef UART_HELPERS_H
#define UART_HELPERS_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

static inline void uart_init(unsigned long baud)
{
    unsigned int ubrr = (F_CPU / 16UL / baud) - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0);                  // alleen TX nodig voor monitoren
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

static inline void uart_tx(char c)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = c;
}

static void uart_print(const char *s)
{
    while (*s)
        uart_tx(*s++);
}

static void uart_println(const char *s)
{
    uart_print(s);
    uart_tx('\r');
    uart_tx('\n');
}

static void uart_print_u16(unsigned int v)
{
    char buf[6];
    itoa(v, buf, 10);
    uart_print(buf);
}

static void uart_print_f(float f, int decimals)
{
    // eenvoudige float printer (positief)
    if (f < 0)
    {
        uart_tx('-');
        f = -f;
    }
    unsigned long ipart = (unsigned long)f;
    float frac = f - (float)ipart;
    char buf[12];
    itoa((int)ipart, buf, 10);
    uart_print(buf);
    if (decimals > 0)
    {
        uart_tx('.');
        for (int i = 0; i < decimals; i++)
        {
            frac *= 10.0f;
            int digit = (int)frac;
            uart_tx('0' + digit);
            frac -= digit;
        }
    }
}

#endif
