
// Median Filter
// Es handelt sich hier um ein Bubble sort Allgorythmus, 
// das dann den mittleren Wert aus dem Array zurück gibt.

// es sollte nur eine ungerade Anzahl von Werten verwendet werden

// n = grösse von array
// Aus Effizienzgründen sind 8 und 16Bit getrennt



uint8_t Median_Filter_8Bit(uint8_t arr[], uint8_t n);

uint16_t Median_Filter_16Bit(uint16_t arr[], uint8_t n);

// Beispiel

/*

uint8_t	gefiltert;
uint8_t	Buffer[5];

gefiltert	=	Median_Filter(Buffer,5);

*/