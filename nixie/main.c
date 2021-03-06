/*
 * nixie.c
 *
 * Created: 12/26/2019 8:33:28 PM
 * Author : Nathan
 */ 

// should redo this in  c++

#include <avr/io.h>
#include <stdlib.h>

#define F_CPU 1000000
#include <util/delay.h>

#define HC595 PORTC
#define HC595_DDR DDRC
#define HC595_DATA PORTC2
#define HC595_CLOCK PORTC1
#define HC595_LATCH PORTC0

void hc595_clock_pulse(void)
{
	HC595 |= 1<<HC595_CLOCK;
	HC595 &= ~(1<<HC595_CLOCK);
}

void hc595_latch_pulse(void)
{
	HC595 |= 1<<HC595_LATCH;
	_delay_loop_1(1);
	HC595 &= ~(1<<HC595_LATCH);
	_delay_loop_1(1);
}

void shift_bytes_msb(uint8_t bytes[], unsigned int numberOfBytes)
{
	uint8_t data = 0;
	
	for (unsigned int b = 0; b < numberOfBytes; b++)
	{
		data = bytes[b];
		for (uint8_t i = 0; i < 8; i++)
		{
			if (data & 0x80)
			{
				HC595 |= 1<<HC595_DATA;
			}
			else
			{
				HC595 &= ~(1<<HC595_DATA);
			}
		
			hc595_clock_pulse();
		
			data<<=1; // pretty sure this doesn't flow into the next element left of it.
		}
	}
	
	hc595_latch_pulse();
}

void set_tube_digit(uint8_t bytes[], uint8_t digit, unsigned int tube)
{
	// no bounds check done
	bytes[tube-1] = digit;
}

void display(uint8_t bytes[], unsigned int numberOfBytes)
{
	unsigned int squishedBytesSize = 0;
	
	if (numberOfBytes % 2 == 0) //even
	{
		squishedBytesSize = numberOfBytes/2;
	}
	else // odd
	{
		squishedBytesSize = (numberOfBytes+1)/2;
	}
	
	// squish the array into half of its size since 1 74HC595 controls 2 K155ID1
	for (unsigned int i = 0; i < numberOfBytes; i++)
	{
		// no bounds checking on going over display bytes size, better hope its correct.
		// on odd elements, shift it left 4 and put it in the same byte as the previous element.
		// on even elements, 
		if (i%2 == 0) // even
		{
			bytes[i/2] = bytes[i];
		}
		
		else // odd
		{
			bytes[(i-1)/2] |= bytes[i]<<4;
		}
	}
	
	shift_bytes_msb(bytes, squishedBytesSize);
}

// nixie clock driver might already have this functionality.
void scroll(unsigned int numberOfTubes)
{
	uint8_t scrollBytes[numberOfTubes];
	
	for (uint8_t j = 0; j <= 9; j++) // scroll from 0 to 9 for each tube.
	{
		for (unsigned int k = 0; k < numberOfTubes; k++)
		{
			scrollBytes[k] = j;
		}
		
		display(scrollBytes, numberOfTubes);
		_delay_ms(500);
	}
}

//// need this?
//uint8_t get_tube_digit(uint8_t bytes[], unsigned int tube)
//{
	//return bytes[tube];
//}
//
//void count(unsigned int until)
//{
	//// should error check that number until can be display by number of tubes.
	//
	//
//}

#define OFF 0xF

int main(void)
{
	// Set control lines to output.
	DDRC = 1<<HC595_DATA | 1<<HC595_CLOCK | 1<<HC595_LATCH;
	
	const int NumberOfTubes = 4;
	
	// Do I want some kind of init here?
	//hc595_clock_pulse();
	//hc595_latch_pulse();
	
	// Really this should all be encapsulated in a class buuut not sure how to set the #defines properly to PORTX DDRX etc.
	//scroll(NumberOfTubes);
	// delay needed here for scroll? 
	// delay_ms(10000); // 10 seconds
	
	uint8_t nixie[NumberOfTubes];
	//set_tube_digit(nixie, 2, 1); // tubes are numbered naturally starting at 1, function accounts for this.
	//set_tube_digit(nixie, 0, 2);
	//set_tube_digit(nixie, 1, 3);
	//set_tube_digit(nixie, 9, 4);
	
	// for turning off set anything from 10-15
	set_tube_digit(nixie, 0xF, 1); // tubes are numbered naturally starting at 1, function accounts for this.
	set_tube_digit(nixie, 0xF, 2);
	set_tube_digit(nixie, 0xF, 3);
	set_tube_digit(nixie, 0xF, 4);
	
	
    	while (1) 
	{
		display(nixie, NumberOfTubes);
		_delay_ms(1000);
		scroll(NumberOfTubes);
		
		for (uint8_t n = 0; n<10;)
		{
			set_tube_digit(nixie, n, 4);
			n++;
			display(nixie, NumberOfTubes);
			_delay_ms(500);
			set_tube_digit(nixie, OFF, 4);
			display(nixie, NumberOfTubes);
			
			set_tube_digit(nixie, n, 3);
			n++;
			display(nixie, NumberOfTubes);
			_delay_ms(500);
			set_tube_digit(nixie, OFF, 3);
			display(nixie, NumberOfTubes);
		}
		
    	}
}

