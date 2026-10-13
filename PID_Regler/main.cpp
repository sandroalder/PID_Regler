/*
 * PID_Regler.cpp
 *
 * Created: 21.07.2023 11:39:47
 * Author : SA061401
 */ 

#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define	F_CPU 20000000
#include <util/delay.h>

extern "C"
{
	#include "SPI.h"
	#include "median_filter.h"
	#include "lcd-routines.h"
	#include "Ultra_Sonic_Sensor.h"
};

// Interrupt Vectoren für Ultraschallsensor
// Müssen in main File sein
ISR(TCA0_CMP0_vect)
{
Ultra_Sonic_Sensor_TCA0_CMP_Interrupt();
}
ISR(PORTB_PORT_vect)
{
Ultra_Sonic_Sensor_Pin_Change_Interrupt();
}


void clock_init (void)
{
	// Ermögliche das schreiben in geschützte Register
	CPU_CCP = CCP_IOREG_gc;
	// Wähle externe clock source
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;
	// Ermögliche das schreiben in geschützte Register
	CPU_CCP = CCP_IOREG_gc;
	// Schalte Prescaler aus
	CLKCTRL.MCLKCTRLB &=  ~CLKCTRL_PEN_bm;
	// Warte bis sich der clock umgestellt hat
	while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm){}	
}

void ADC_init (void)
{
	// Schalte digitale Input Buffer aus
	PORTE.PIN0CTRL &= ~PORT_ISC_gm;
	PORTE.PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTE.PIN1CTRL &= ~PORT_ISC_gm;
	PORTE.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTE.PIN2CTRL &= ~PORT_ISC_gm;
	PORTE.PIN2CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTE.PIN3CTRL &= ~PORT_ISC_gm;
	PORTE.PIN3CTRL |= PORT_ISC_INPUT_DISABLE_gc;	
			
	// 8 Bit mode / Aktiviere ADC
	ADC0_CTRLA	=	ADC_RESSEL_8BIT_gc
				|	ADC_ENABLE_bm;

	// AVDD als Referenz / Prescaler 8 
	// (clock darf nicht über eine bestimmte Frequenz sonst geht es nicht.
	// Ist im DB leider unzureichend dokumentiert)
	ADC0_CTRLC	=	ADC_REFSEL_VDDREF_gc
				|	ADC_PRESC_DIV8_gc;
}

//  ADC Wert einlesen und filtern
uint8_t ADC_read_and_filter()
{
	uint8_t	ADC_gefiltert;
	uint8_t	Buffer[5];
	
	for(uint8_t i=0; i<5; i++)
	{		// starte ADC		ADC0.COMMAND = ADC_STCONV_bm;		// warte bis conversion abgeschlossen		while (ADC0.COMMAND & ADC_STCONV_bm){}
		Buffer[i]	=	ADC0.RESL;
	}
	ADC_gefiltert	=	Median_Filter_8Bit(Buffer,5);
	return ADC_gefiltert;
}

// Gebe hier die neuen Werte aus
void lcd_update (	uint8_t P, 
					uint8_t I, 
					uint8_t D,
					uint16_t Soll, 
					uint16_t Ist)
{
	char Buffer[10];
	
	lcd_home();
	lcd_string("P");
	if (P<100)
		lcd_string("0");
	if (P<10)
		lcd_string("0");
	utoa( P, Buffer, 10 );
	lcd_string(Buffer);
	
	lcd_setcursor(5,1);
	lcd_string("I");
	if (I<100)
		lcd_string("0");
	if (I<10)
		lcd_string("0");
	utoa( I, Buffer, 10 );
	lcd_string(Buffer);
	
	lcd_setcursor(10,1);
	lcd_string("D");
	if (D<100)
		lcd_string("0");
	if (D<10)
		lcd_string("0");
	utoa( D, Buffer, 10 );
	lcd_string(Buffer);
	
	lcd_setcursor(0,2);
	lcd_string("S");
	if (Soll<10000)
		lcd_string("0");
	if (Soll<1000)
		lcd_string("0");
	if (Soll<100)
		lcd_string("0");
	if (Soll<10)
		lcd_string("0");
	utoa( Soll, Buffer, 10 );
	lcd_string(Buffer);	
	
	lcd_setcursor(8,2);
	lcd_string("I");
	if (Ist<10000)
		lcd_string("0");
	if (Ist<1000)
		lcd_string("0");
	if (Ist<100)
		lcd_string("0");
	if (Ist<10)
		lcd_string("0");
	utoa( Ist, Buffer, 10 );
	lcd_string(Buffer);	
}


int main(void)
{
	uint8_t P = 0;
	uint8_t I = 0;
	uint8_t D = 0;
	
	uint16_t Soll = 0;
	uint16_t Ist = 0;
	
	clock_init();
	ADC_init();
	lcd_init();
	Ultra_Sonic_Sensor_init();
	
	// CS für SPI muss vor SPI init gemacht werden. Sonst geht es nicht.
	// Ist im Datenblatt leider unzureichend dokumentiert
	PORTA.DIR = PIN7_bm;
	PORTA.OUT |= PIN7_bm;
	SPI0_init_Host();

	
	// aktiviere globale Interrupts
	sei();
	
    while (1)
    {	
		// Lese P Wert aus ADC und invertiere
		ADC0.MUXPOS = ADC_MUXPOS_AIN11_gc;
		P = ~ADC_read_and_filter();
		
		// Lese I Wert aus ADC und invertiere
		ADC0.MUXPOS = ADC_MUXPOS_AIN10_gc;
		I = ~ADC_read_and_filter();
		
		// Lese D Wert aus ADC und invertiere
		ADC0.MUXPOS = ADC_MUXPOS_AIN9_gc;
		D = ~ADC_read_and_filter();
		
		// Lese Soll Wert aus ADC und invertiere
		ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc;
		Soll = (~ADC_read_and_filter() & 0x0F);
			
		Ist = Ultra_Sonic_Sensor_read();
		lcd_update(P,I,D,Soll,Ist);
		
		PORTA.OUT &= ~PIN7_bm;
		_delay_us(5);
		SPI0_exchange_data(P);
		PORTA.OUT |= PIN7_bm;
		
		_delay_ms(1);	
	
    }
	return 0; 
}
