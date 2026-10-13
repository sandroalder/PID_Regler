
//https://ww1.microchip.com/downloads/en/Appnotes/TB3217-Getting-Started-with-TCA-DS90003217.pdf


#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include "Ultra_Sonic_Sensor.h"
#include <util/delay.h>


void Ultra_Sonic_Sensor_init( void )
{
	// Aktiviere Externen Interrupt auf beide Flanken von Echo Signal
	Echo_CTRL |= PORT_ISC_BOTHEDGES_gc;
	
	// Schalte Trig als Ausgang 
	Trig_Dir	|=	Trig;
	Trig_Out	|=	Trig;
	
	//config Timer
	
	// select prescaler
	// mit prescaler 16 ein increment alle 0.8uS bei einem 20MHz Takt
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV16_gc;
	
	// Aktiviere Interrupt "Compare Match 0"
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0EN_bm;
	
	// Zähle clock ticks
	TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
	
	// Compare Match Interrupt nach 50mS
	TCA0_SINGLE_CMP0 = 62500;
		
	// starte Timer
	TCA0.SINGLE.CTRLA |=  TCA_SINGLE_ENABLE_bm;
}


void Ultra_Sonic_Sensor_TCA0_CMP_Interrupt(void)
{
	//lösche Interrupt flag
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP0EN_bm;
	
	//Reset Counter Register
	TCA0.SINGLE.CNT	=	0;
	
	// Start signal für 10uS
	Trig_Out &= ~Trig;
	_delay_us(10);
	Trig_Out |= Trig;
}


uint16_t RohWert;
uint16_t RohWert_Buffer;

void Ultra_Sonic_Sensor_Pin_Change_Interrupt(void)
{
	// lösche Interrupt flag
	Echo_INTFL = PORT_INT2_bm;
	
	// Unterscheide zwischen positiver und negativer Flanke
	if(Echo_IN & Echo)
	{
		RohWert_Buffer = TCA0.SINGLE.CNT;
	}
	else
	{
		RohWert = (TCA0.SINGLE.CNT - RohWert_Buffer);
	}
}

uint16_t Ultra_Sonic_Sensor_read (void)
{
	uint16_t Distanz;
	
	Distanz = RohWert;
	// 10 increment pro uS
	Distanz /= 12.5;
	
	// berechne Distanz in Zehntel Millimeter
	//	34.3 Schallgeschwindigkeit
	//	2 mal die Strecke
	Distanz *= (34.3 / 2);
	
	return Distanz;
}
