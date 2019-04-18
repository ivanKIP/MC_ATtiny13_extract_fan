/*******************************************************
This program was created by the CodeWizardAVR V3.33
Automatic Program Generator
© Copyright 1998-2018 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : Вентилятор ванна+туалет
Version : 1.1
Date    : 01.04.2019
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
#include <delay.h>
#include <math.h>

// Global variables
#define IN_V PINB.3             // Вход №1 (ванная)
#define IN_T PINB.4             // Вход №2 (туалет)
#define OUT PORTB.0             // Выход 1 (вкл. вентилятор)
#define SELF_PWR PORTB.1        // Выход 2 (вкл. подхват питания)
#define DELAY_TO_ON_V 40        // Задержка от вкл. света в ванной (IN_V) до вкл. вентилятора (сек.)
#define DELAY_TO_ON_T 20        // Задержка от вкл. света в туалете (IN_T) до вкл. вентилятора (сек.)
#define DELAY_TO_OFF_V 210      // Задержка после выкл. света в ванной до выкл. вентилятора (сек.)
#define DELAY_TO_OFF_T 80       // Задержка после выкл. света в туалете до выкл. вентилятора (сек.)
unsigned int cnt = 0;           // Счетчик "тиков" (переполнений таймера)
unsigned char in_v_cnt = 0;     // Счетчик переключений IN_V
unsigned char in_t_cnt = 0;     // Счетчик переключений IN_T
bit in_v_rev = 0;               // Флаг переключения IN_V
bit in_t_rev = 0;               // Флаг переключения IN_T
bit in_v_on = 0;                // Признак включения IN_V
bit in_t_on = 0;                // Признак включения IN_T
bit in_v_after_t = 0;           // Признак включения света в ванной после туалета
unsigned int time_cnt = 0;      // Счетчик секунд
unsigned char last_off = 0;     // Метка последнего выключенного (1 - IN_V; 2 - IN_T)
bit time_cnt_res = 0;           // Флаг сброса счетчика секунд

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
	// При переполнении начинаем заново с 0x6A (каждое переполнение = 1 мс.)
	// и инкрементируем счетчик переполнений cnt
	TCNT0 = 0x6A;
	cnt++;
}

void stop_timer(void)
{
	if (TIMSK0 != 0x00) {
		TIMSK0 = 0x00;
	}
}

void start_timer(void)
{
	if (TIMSK0 == 0x00) {
        cnt = 0;
		TCNT0 = 0x6A;
		TIFR0 = 0x00;
		TIMSK0 = 0x02;
	}
}

void check_in_v(void)
{
    if (IN_V && !in_v_rev) {
        in_v_cnt++;
        in_v_rev = 1;
    }
    if (!IN_V && in_v_rev) {
        in_v_rev = 0;
    }
    delay_us(100);
}
    
void check_in_t(void)
{
    if (IN_T && !in_t_rev) {
        in_t_cnt++;
        in_t_rev = 1;
    }
    if (!IN_T && in_t_rev) {
        in_t_rev = 0;
    }
    delay_us(100);
}

// Отключение питания с ожиданием завершения переходных процессов
void power_off(void)
{
    stop_timer();
    SELF_PWR = 0;
    cnt = 0;
    time_cnt = 0;
    in_v_after_t = 0;
    time_cnt_res = 0;
    in_v_cnt = 0;
    in_t_cnt = 0;
    delay_ms(2000);
//    #asm("sleep");
}

void main(void)
{
    // Максимальная задержка на включение
    unsigned int max_delay_to_on = max(DELAY_TO_ON_V, DELAY_TO_ON_T);
    
	// Crystal Oscillator division factor: 8
    #pragma optsize-
	CLKPR = (1 << CLKPCE);
	CLKPR = (0 << CLKPCE) | (0 << CLKPS3) | (0 << CLKPS2) | (1 << CLKPS1) | (1 << CLKPS0);
	#ifdef _OPTIMIZE_SIZE_
    #pragma optsize+
	#endif

	// Input/Output Ports initialization
	// Port B initialization
	// Function: Bit5=In Bit4=In Bit3=In Bit2=In Bit1=Out Bit0=Out
	DDRB = (0 << DDB5) | (0 << DDB4) | (0 << DDB3) | (0 << DDB2) | (1 << DDB1) | (1 << DDB0);
	// State: Bit5=T Bit4=T Bit3=T Bit2=T Bit1=0 Bit0=0
	PORTB = (0 << PORTB5) | (0 << PORTB4) | (0 << PORTB3) | (0 << PORTB2) | (0 << PORTB1) | (0 << PORTB0);

	// Timer/Counter 0 initialization
	// Clock source: System Clock
	// Clock value: 150,000 kHz
	// Mode: Normal top=0xFF
	// OC0A output: Disconnected
	// OC0B output: Disconnected
	// Timer Period: 1 ms
	TCCR0A = (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (0 << WGM01) | (0 << WGM00);
	TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (0 << CS00);
	TCNT0 = 0x6A;
	OCR0A = 0x00;
	OCR0B = 0x00;

	// Timer/Counter 0 Interrupt(s) initialization
	TIMSK0 = (0 << OCIE0B) | (0 << OCIE0A) | (0 << TOIE0);

	// External Interrupt(s) initialization
	// INT0: Off
	// Interrupt on any change on pins PCINT0-5: Off
	GIMSK = (0 << INT0) | (0 << PCIE);
    // Разрешаем сон, режим - PowerDown
	MCUCR = (0 << ISC01) | (0 << ISC00) | (1 << SE) | (1 << SM1);

	// Analog Comparator initialization
	// Analog Comparator: Off
	// The Analog Comparator's positive input is
	// connected to the AIN0 pin
	// The Analog Comparator's negative input is
	// connected to the AIN1 pin
	ACSR = (1 << ACD) | (0 << ACBG) | (0 << ACO) | (0 << ACI) | (0 << ACIE) | (0 << ACIS1) | (0 << ACIS0);
	ADCSRB = (0 << ACME);
	// Digital input buffer on AIN0: On
	// Digital input buffer on AIN1: On
	DIDR0 = (0 << AIN0D) | (0 << AIN1D);

	// ADC initialization
	// ADC disabled
	ADCSRA = (0 << ADEN) | (0 << ADSC) | (0 << ADATE) | (0 << ADIF) | (0 << ADIE) | (0 << ADPS2) | (0 << ADPS1) | (0 << ADPS0);
                 
    // Включаем подхват питания
    SELF_PWR = 1;

	// Globally enable interrupts
    #asm("sei")

	while (1) { 

        start_timer();
        check_in_v();
        check_in_t();
        
        // Каждые 1000 тиков таймера (~1 сек.)
        if (cnt >= 1000) {
            stop_timer();
            time_cnt++;
            
            // Проверяем IN-V на включение
            // если кол-во переключения > 45 (частота сети 50 Гц), то включен
            // инчае - выключен
            if (in_v_cnt >= 45) {
                in_v_on = 1;
            } else {
                if (in_v_on && !in_t_on) {
                    last_off = 1;
                    // Если свет в ванной был включен и выключен после туалета
                    // и время на включение вентилятора для ванной еще не вышло,
                    // то оставляем признак последнего выключенного для туалета
                    if (in_v_after_t && time_cnt < DELAY_TO_ON_V) {
                        last_off = 2;
                    }
                }
                in_v_on = 0;
            }
            
            // Проверяем IN-T на включение
            // если кол-во переключения > 45 (частота сети 50 Гц), то включен
            // инчае - выключен
            if (in_t_cnt >= 45) {
                in_t_on = 1;
                in_v_after_t = 0;
            } else {
                if (in_t_on && !in_v_on) {
                    last_off = 2;
                }
                in_t_on = 0;
            }
            
            // Сбрасываем счетчики переключений
            in_v_cnt = 0;
            in_t_cnt = 0;
            
            // Отключение схемы если свет отключили до истечения задержки включения
            if (!OUT && !in_v_on && !in_t_on && time_cnt < max_delay_to_on) {
                power_off();
            }
        }
                    
        // Если включен любой свет,
        // сбрасываем признак последнего выключенного
        // и флаг сброса счетчика секунд
        if (in_v_on || in_t_on) {
            // Если мы уже выключили самоподхват - включаем его
            if (!SELF_PWR) {
                SELF_PWR = 1;
            }
            // Если включили свет в ванной сразу после туалета -
            // устанавливаем соответствующий признак и сбрасываем счетчик секунд
            if (in_v_on && !in_v_after_t && last_off == 2) {
                in_v_after_t = 1;
                time_cnt = 0;
            } else if (in_v_on && in_v_after_t && last_off == 2) {
                in_v_after_t = 0;
            }
            time_cnt_res = in_v_after_t;
            last_off = 0;
        }

        // Включение вентилятора
        // если он не включен,
        // включен свет в ванной
        // и счетчик секунд уже больше задержки включения
        if (!OUT && in_v_on && time_cnt >= DELAY_TO_ON_V) {
            OUT = 1;
        }
        
        // Включение вентилятора
        // если он не включен,
        // включен свет в туалете
        // и счетчик секунд уже больше задержки включения
        if (!OUT && in_t_on && time_cnt >= DELAY_TO_ON_T) {
            OUT = 1;
        }

        // Если есть признак последнего выключенного
        // и счетчик секунд не сброшен - 
        // сбрасываем счетчик секунд и устанавиливаем флаг сброса счетчика секунд
        if ((last_off == 1 || last_off == 2) && !time_cnt_res) {
            time_cnt = 0;
            time_cnt_res = 1;
        } 
        
        // Отключение вентилятора и самой схемы: 
        // если вентилятор включен,
        // есть признак выключенного
        // и счетчик секунд больше необходимой задержки
        
        if (OUT && last_off == 1 && time_cnt >= DELAY_TO_OFF_V) {
            OUT = 0;
            power_off();
        }
        
        if (OUT && last_off == 2 && time_cnt >= DELAY_TO_OFF_T) {
            OUT = 0;
            power_off();
        }
	}
}
