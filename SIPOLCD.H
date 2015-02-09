#ifndef _SIPO_LCD_
#define _SIPO_LCD_ 1

#include<stdarg.h>
#include<util/delay.h>

#include "ds.h" //contains my lib functions related data structures like stack, queue etc



/* Program to display a number on the lcd without using sprintf or cmd 0x04
	so as to maximize memory usage in an efficient way by the help of a stack.

	Author : N.Srinivas (a.k.a NeutroN StrikeR)

	Date : 19.06.2014

	email: striker.dbz[at]hotmail.com

	The MIT License (MIT)

Copyright (c) 2014 N.Srinivas a.k.a. NeutroN StrikeR

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

	*/


#ifdef SIPO_PORT //by doing this i just define the PORT to which SIPO LCD is
				//connected and PORT and DDR are defined automatically

				//also added a line in LCD Init function to set SIPO pins
				//as output

#define SIPO_CONCAT(a, b)   a ## b
#define SIPO_D(x) SIPO_CONCAT(DDR,x)
#define SIPO_P(x) SIPO_CONCAT(PORT,x)

#define SIPO     SIPO_P(SIPO_PORT)
#define SIPODDR  SIPO_D(SIPO_PORT)

#endif

#ifndef SIPO_PORT
#define SIPO PORTB
#define SIPODDR DDRB
#warning SIPO Connection port has not been defined, taking default as PORTB
#endif

/*
#ifndef SIPO
#define SIPO PORTB
#warning SIPO Connection port has not been defined, taking default as PORTB
#endif
*/
#ifndef STB
#define STB 3
#warning SIPO STB pin has not been defined, taking default as bit 3 of SIPO port
#endif

#ifndef SPIN
#define SPIN 4
#warning SIPO SPIN pin has not been defined, taking default as bit 4 of SIPO port
#endif

#ifndef CLK
#define CLK 5
#warning SIPO CLK pin has not been defined, taking default as bit 5 of SIPO port
#endif




#define en 2 //q3 of SIPO
#define rs 0 //q1 of SIPO

void SIPO_send(unsigned char);
void send_nibble(unsigned char);
void lcd_init();
void print(char *, unsigned char);

void lcd_cmd(unsigned char);
void lcd_data(unsigned char);
void lcd_printnum(long int);
void lcd_clear();

//for compatibility with old programs which don't know the new names written above
void printnum(long int);
void cmd(unsigned char);
void data(unsigned char);

//Wrapper functions for compatibility
void cmd(unsigned char val)
{
	lcd_cmd(val);
}

void data(unsigned char val)
{
	lcd_data(val);
}

void printnum(long int number)
{
	lcd_printnum(number);
}


// I could have used the #defines but the problem is that if use "#define cmd lcd_cmd" then during compiling an error will come stating
//"function with name lcd_lcd_cmd not declared" This would have because the old name is also a part of the new name and it would lead to
//erronous infinite loops



/* This program demonstrates True 3 pin operation of LCD using the strobe pin, serial in pin, and CLK pin
and all pins of lcd are directly connected to SIPO only. The data for enable and RS are also coupled
into the SIPO and the lCD is driven in 4 bit mode. The strobe pin is utilised to send the enable CLK to
the lcd as described in the working below.

Author : N.Srinivas a.k.a NeutroN StrikeR

Date : 01-08-2014
											*/


void SIPO_send(unsigned char var) // you may try to make these functions inline so that this might give a performance
{									//boost by decreasing use of pointers and ram. but this will increase however increase
									//size of the program, you have to keep that in mind.
	unsigned char i;

	for(i=0;i<8;i++)
	{
		if(var & (1<<7))
		SIPO |= (1<<SPIN);

		else
		SIPO &= ~(1<<SPIN);

		SIPO |= (1<<CLK);
		_delay_us(1); //can be reduced to 1us i have checked it
					//so our SIPO clock is 1/10us that is 100khz speed
					//80 us delay for 1 byte
		SIPO &= ~(1<<CLK);
		_delay_us(1);

		var = var<<1;
	}

}

void send_nibble(unsigned char x)  //try to make this also inline if necessary
{
	x |= (1<<en); // set en pin as 1

	SIPO_send(x); //send the data through SIPO

	SIPO |= (1<<STB); //after sending enable the STROBE so that data is now available on the data pins

	_delay_ms(1); //give delay of 10ms for enable change CLK

	SIPO &= ~(1<<STB);//clear the strobe now

	x &= ~(1<<en); // now make the enable pin of lcd as 0

	SIPO_send(x); // again send new data in which only enable pin state of lcd is changed

	SIPO |= (1<<STB); //again set the strobe
	_delay_ms(1); //delay 10ms
	SIPO &= ~(1<<STB); // clear the strobe


		//to send a nibble 2 * 80us + 2 * 1ms = 2.16ms total delay
}

void lcd_cmd(unsigned char var)
{
	unsigned char x;

//send higher nibble
	x = 0xf0 & var; //get higher nibble into x

	x &= ~(1<<rs); 	//set the rs pin as 0
	send_nibble(x); //enable pin clocking mechanism is contained in nibble fucntion

//now send lower nibble
	x = 0xf0 & (var<<4);   // now repeat same process for lower nibble

	x &= ~(1<<rs);
	send_nibble(x);

//to send a cmd or data byte delay is 2 * 2.16ms = 4.32ms

//so data rate is 1/4.32ms = 231 char's /second ; good enough for our display
}




void lcd_data(unsigned char var)
{
	unsigned char x;

//send higher nibble
	x = 0xf0 & var;

	x |= (1<<rs);
	send_nibble(x);

//now send lower nibble
	x = 0xf0 & (var<<4);

	x |= (1<<rs);
	send_nibble(x);


}

void lcd_init()
{

//setting SIPO pins as output
SIPODDR |= (1<<STB) | (1<<SPIN) | (1<<CLK);

//proper initialization sequence
	_delay_ms(200); //wait until lcd's internal startup finished
	cmd(0x03);//first command
	_delay_ms(5);
	cmd(0x03);//second command
	_delay_us(50);
	cmd(0x03);//third command
	_delay_us(50);
	cmd(0x02);//set 4 bit mode interface
	_delay_us(50);
	cmd(0x28);//the real function set command
	_delay_us(50);
	cmd(0x08);//turn off display cursor, blink
	_delay_us(100);
	cmd(0x01);//clear display
	_delay_ms(5);//allow enough time to write zeros to the whole ddram and cgram
	cmd(0x06);//entry mode set
	_delay_us(50);
	cmd(0x0e);//turn on display with 1DCB
	_delay_us(50);
	cmd(0x80);//set ddram address
	_delay_us(50);
}


void lcd_clear()
{
	lcd_cmd(0x01);
}

void lcd_print(char* msg,...)
{
	int x;
	va_list arg_list;//declare "arg_list" a variable of type va_list
	va_start(arg_list,msg);//initialize the variable with "va_start" which takes
							//two arguments the list and first argument in this function
	x=va_arg(arg_list,int);//when va_arg is called with the argument of list and a datatype
						// it returns next value of that type in the list if it exists
						//if it doesn't exist if returns NULL or 0 (tested in Turbo C)
	if(x>=0x80 && x<=0xcf)//so i don't want to check because i only want one argument to be
	print(msg,x);	//optional so i directly get the address and specify which should be
						//the next argument in the calling function(caller)

	else
	print(msg,0x80);

	va_end(arg_list); //clear the memory used by arg_list variable
}



void print(char* sym,unsigned char startadd)
{
	unsigned char i=0;

	if(startadd<0x80 || startadd>0xcf)
		return;
	else
		cmd(startadd);

	while(sym[i]!='\0')
	{

		if(startadd==0x90)
		{
				cmd(0xc0);
				startadd=0xc0;
		}
		else if(startadd==0xD0)
		{
			cmd(0x01);
			startadd=0x80;
		}

		data(sym[i]);
		i++;
		startadd++;
	}

}


void lcd_printnum(long int number)
{
	unsigned char temp=0;

	if(number == 0)
	{
		lcd_data('0');
		return;
	}

	if(number < 0)
	{
		number = number * -1;
		lcd_data('-');
	}

	while(number > 0)
	{
		temp = number%10;
		temp += 0x30;
		push(temp);
		number = number/10;
	}

	for(int8_t i=top;i>=0;i--)
		lcd_data(stack[i]);

	top = -1; // re-initialize stack pointer

}



#endif
