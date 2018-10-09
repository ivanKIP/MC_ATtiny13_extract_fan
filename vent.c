/*******************************************************
This program was created by the CodeWizardAVR V3.33 
Automatic Program Generator
© Copyright 1998-2018 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : Вентилятор ванна+туалет
Version : 1.0
Date    : 17.09.2018
Author  : ivan_kip
Company : 
Comments: 


Chip type               : ATtiny13A
AVR Core Clock frequency: 1,200000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 16
*******************************************************/

#include <tiny13a.h>

// Declare your global variables here
#define IN1 PINB.3              //вход №1 (ванная)
#define IN2 PINB.4              //вход №2 (туалет)
#define OUT PORTB.0             //выход 1 (вкл. вентилятор)
#define SELF_PWR PORTB.1        //выход 2 (вкл. подхват питания)
unsigned int cnt = 0;           //счетчик "тиков"

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    //при переполнении начинаем заново с 0x6A (каждое переполнение = 1 мс.)
    //и инкрементируем счетчик переполнений cnt
    TCNT0=0x6A;
    cnt++;
}

void stop_timer(void) {
    if (TIMSK0 == 0x02) {
        TIMSK0 = 0x00;
    }
}

void start_timer(void) {
    if (TIMSK0 == 0x00) {
        TCNT0 = 0x6A;
        TIFR0 = 0x00;
        TIMSK0 = 0x02;
    }
}

void main(void)
{
    // Declare your local variables here

    // Crystal Oscillator division factor: 8
    #pragma optsize-
    CLKPR=(1<<CLKPCE);
    CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (1<<CLKPS1) | (1<<CLKPS0);
    #ifdef _OPTIMIZE_SIZE_
    #pragma optsize+
    #endif

    // Input/Output Ports initialization
    // Port B initialization
    // Function: Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=Out 
    DDRB=(0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (0<<DDB2) | (0<<DDB1) | (1<<DDB0);
    // State: Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=0 
    PORTB=(0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: 150,000 kHz
    // Mode: Normal top=0xFF
    // OC0A output: Disconnected
    // OC0B output: Disconnected
    // Timer Period: 1 ms
    TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
    TCCR0B=(0<<WGM02) | (0<<CS02) | (1<<CS01) | (0<<CS00);
    TCNT0=0x6A;
    OCR0A=0x00;
    OCR0B=0x00;

    // Timer/Counter 0 Interrupt(s) initialization
    TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (1<<TOIE0);

    // External Interrupt(s) initialization
    // INT0: Off
    // Interrupt on any change on pins PCINT0-5: Off
    GIMSK=(0<<INT0) | (0<<PCIE);
    MCUCR=(0<<ISC01) | (0<<ISC00);

    // Analog Comparator initialization
    // Analog Comparator: Off
    // The Analog Comparator's positive input is
    // connected to the AIN0 pin
    // The Analog Comparator's negative input is
    // connected to the AIN1 pin
    ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIS1) | (0<<ACIS0);
    ADCSRB=(0<<ACME);
    // Digital input buffer on AIN0: On
    // Digital input buffer on AIN1: On
    DIDR0=(0<<AIN0D) | (0<<AIN1D);

    // ADC initialization
    // ADC disabled
    ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);


    // Globally enable interrupts
    #asm("sei")

    while (1) {
        // Place your code here
        if (IN1 || IN2) {
            OUT = 1;
        } else {
            OUT = 0;
        }
        
        if (cnt == 10) {
            stop_timer();
        }
        
        if (cnt == 20) {
            start_timer();
        }
    }
}
