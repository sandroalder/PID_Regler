// Ansteuerubg eines Ultraschall Sensors 

// Das Programm ist auf einen 20MHz Takt ausgelegt
// Wird eine andere Taktrate verwendet muss das Programm entsprechend angepasst werden

// Der Timer Counter A und der entsprechende Port Interrupt Vector werden für dieses 
// Programm gebraucht und können nicht mehr anders verwendet werden.

// Interrupts müssen im main Programm aktiviert werden. mit sei(); und cli();




// Taktrate eintragen
#define	F_CPU 20000000

// lege Pins fest
 
#define Trig_Dir	PORTB.DIR
#define Trig_Out	PORTB.OUT
#define Trig		PIN1_bm

#define Echo_Dir	PORTB.DIR
#define Echo_CTRL	PORTB.PIN2CTRL
#define Echo_INTFL	PORTB.INTFLAGS
#define Echo_IN		PORTB.IN
#define Echo		PIN2_bm


void Ultra_Sonic_Sensor_init(void);

uint16_t Ultra_Sonic_Sensor_read(void);

void Ultra_Sonic_Sensor_TCA0_CMP_Interrupt(void);

void Ultra_Sonic_Sensor_Pin_Change_Interrupt(void);

/* Beispiel

// ISR müssen in main File sein

ISR(TCA0_CMP0_vect)
{
	Ultra_Sonic_Sensor_TCA0_CMP_Interrupt();
}

ISR(PORTB_PORT_vect)
{
	Ultra_Sonic_Sensor_Pin_Change_Interrupt();
}

int main(void)
{
	uint32_t Distanz = 0;
	
	While (1)
	{
		Distanz = Ultra_Sonic_Sensor_read();	
	}
}
*/