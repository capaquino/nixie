/*
 * nixie.c
 *
 * Created: 12/26/2019 8:33:28 PM
 * Author : Nathan
 */ 

// should redo this in  c++

#include <avr/io.h>
#include <stdlib.h>

#define F_CPU 8000000
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
	for (unsigned int b = 0; b < numberOfBytes; b++)
	{
		for (uint8_t i = 0; i < 8; i++)
		{
			if (bytes[b] & 0x80)
			{
				HC595 |= 1<<HC595_DATA;
			}
			else
			{
				HC595 &= ~(1<<HC595_DATA);
			}
		
			hc595_clock_pulse();
		
			bytes[b]<<=1; // pretty sure this doesn't flow into the next element left of it.
		}
	}
	
	hc595_latch_pulse();
}

void set_tube_digit(uint8_t bytes[], uint8_t digit, unsigned int tube)
{
	bytes[tube] = digit;
}

void display(uint8_t bytes[], unsigned int numberOfBytes)
{
	unsigned int displayBytesSize = 0;
	
	
	if (numberOfBytes % 2 == 0) //even
	{
		displayBytesSize = numberOfBytes/2;
	}
	else // odd
	{
		displayBytesSize = (numberOfBytes+1)/2;
	}
	
	uint8_t* displayBytes = malloc(sizeof(uint8_t) * displayBytesSize);
	// no malloc check eh
	
	// squish the array into half of its size since 1 74HC595 controls 2 K155ID1
	for (unsigned int i = 0; i < numberOfBytes; i++)
	{
		// no bounds checking on going over display bytes size, better hope its correct.
		
		// on odd elements, shift it 4 and put it in the same byte as the previous element.
		
		if (i%2 == 0) //even
		{
			displayBytes[i] = bytes[i];
		}
		else // odd
		{
			displayBytes[i-1] |= bytes[i]<<4;
		}
	}
	
	shift_bytes_msb(bytes, numberOfBytes);
	
	free(displayBytes);
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

int main(void)
{
	// Set control lines to output.
	DDRC = 1<<HC595_DATA | 1<<HC595_CLOCK | 1<<HC595_LATCH;
	
	const int NumberOfTubes = 4;
	
	// Really this should all be encapsulated in a class buuut not sure how to set the #defines properly to PORTX DDRX etc.
	scroll(NumberOfTubes);
	
	uint8_t nixie[NumberOfTubes];
	set_tube_digit(nixie, 2, 1);
	set_tube_digit(nixie, 0, 2);
	set_tube_digit(nixie, 1, 3);
	set_tube_digit(nixie, 9, 4);
	display(nixie, NumberOfTubes);
	
    while (1) 
    {
		// can count or do something here.
    }
}

