#define F_CPU 12000000UL

#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include<string.h>          //used for strstr();

#define led_on() DDRB |= (1<<1); PORTB |= (1<<1) //led turn on and off functions
#define led_off() PORTB &= ~(1<<1)



/* This is an example program which demonstrates how to interface with a GSM
Modem for receiving or making a call or receiving or sending a msg and take
some action based on it.

Target Chip: Atmega328p
LCD Module : 16x2 HD44780 based using 3 wire shift-Register mode.
GSM Module : WaveCom GSM modem

Author : N.Srinivas A.K.A. NeutroN StrikeR
Date :  06-09-2014

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


int uart_init(unsigned long int BAUD)
{

	//in this way if i define the uart_init() i can dynamicly switch baud rate
	//in the program on the fly whenever i need.

	unsigned int UBRR_VAL = ((F_CPU/(16UL * BAUD))-1);

	unsigned long int generated_BAUD = (F_CPU/(16UL * (UBRR_VAL+1)));
	int error = 0;
	error = generated_BAUD - BAUD;


	if(error < 0)//taking modulus of error
	error = error * -1;

	error = (error * 100UL)/BAUD;

	if(error >= 2)//acceptable reciever error is +/-2%
		return 0;//i,e, there is more than tolerable error in baud rate generation

	UBRR0L = UBRR_VAL;
	UBRR0H = UBRR_VAL>>8;
	UCSR0B = (1<<TXEN0) | (1<<RXEN0);
	UCSR0C = (1<<UCSZ00) | (1<<UCSZ01); //in atmega328p there is no URSEL bit

	return 1; //serial port initialised succesfully

}


unsigned char uart_read()
{
	unsigned char data;
	while(!(UCSR0A & (1<<RXC0)));
	data= UDR0;
	UCSR0A |= (1<<RXC0);
	return data;
}

void uart_write(unsigned char data)
{
	UDR0=data;
	while(!(UCSR0A & (1<<UDRE0)));
	UCSR0A |= (1<<UDRE0);
}

void u_print(char * string)
{
	uint8_t i=0;
	while(string[i] != '\0')
	{
		uart_write(string[i]);
		i++;
	}
}

#define SIPO_PORT B
#define STB  3
#define SPIN 4
#define CLK  5

#include "sipolcd.h"

char gsm_buffer[100]; //buffer to contain all gsm msgs and notifications
unsigned char buffer_index = 0; //buffer index pointer
unsigned char timer = 0,new_data=0;


int check_response(char * input)
{
	char * resp; //this function searches for the input string in the buffer
				//and returns 1 if it is found.
	resp = strstr(gsm_buffer,input); //strstr() is  case sensitive
	if(resp)
		return 1;
	else return 0;
}

void read_buffer()
{
//this function prints the buffer contents into the lcd

	cmd(0x01);
	data(gsm_buffer[0]);//because 0 % anynumber=0 i.e. 0%31 always = 0 or skip this and instead of i%31==0 & i%15==0 is should have
	//started the for loop from 0 and (i+1)%32==0 and (i+1)%16==0 should have been done
	for(uint8_t i=1;i<buffer_index-1;i++)
	{
		data(gsm_buffer[i]);
		if(i%31==0)
		{
			_delay_ms(2000);
			cmd(0x01);
		}
		else if(i%15==0)
		{
			cmd(0xc0);
		}
	}


}


int gsm_cmd(char * arg)
{
	buffer_index=0;

	cli(); //disable interrupts before the whole string is sent because
			//the modem has local echo enabled

	u_print(arg);

	uart_write('\r'); //after every command either \r carriage return or 0x0D has to be given it is same
					//as pressing enter
					//uart_write(0x0D);

	sei(); //set interrupt again to recieve response

	_delay_ms(500); //delay to allow data to be buffered via RXC interrupt
	if(check_response("OK"))
		return 1;

	else return 0;

//	read_buffer();	//print the data to lcd now

}

void reset_timer()
{
	TCCR0B = (1<<CS00) | (1<<CS02); //clk prescaler of clk/1024
	TCNT0 = 0;
	timer = 0;
}


int gsm_init()
{
	//note uart must be initialised with gsm matching baud before calling this
	//recieve complete interrupt must also be set


	if(!gsm_cmd("AT"))//if this command returns 0 then there is communication error between
		return 0;		//modem and microcontroller or there might be no network
						//as it may give a "NO CARRIER" response


//	gsm_cmd("ATI"); //modem information, this is not necessary for initiation
	gsm_cmd("AT+CLIP=1");//tells the modem to send caller id to UART along with RING
	gsm_cmd("AT+CMGF=1");//tells the modem that sms mode is in TEXT mode not PUD.
	gsm_cmd("AT+CNMI=2,2,0,0");//tells the modem to send the sms text, mobile number and
	//also date and time of receipt instead of just sending a new msg notification

	return 1;

}


int main()
{
	lcd_init();

	if(uart_init(9600)) //the modem is either supporting or configured for 9600 BAUD rate
		lcd_print("UART_initialised");

	else lcd_print("Error initiating UART");

	_delay_ms(1000);

	lcd_print("Please wait initiating GSM...");


	UCSR0B |= (1<<RXCIE0); //turn on USART Receive complete interrupt
	sei();		//turn on global interrupt


	if(gsm_init()) //send the basic commands to the gsm modem required for
	{             //properly receving sms and call number in serial mode
		cmd(0x01);
		lcd_print("GSM Initialised");

	}
	else
	{
		cmd(0x01);
		lcd_print("Error initiating GSM");

	}

	_delay_ms(3000);

	while(1)
	{//this whole part below can be kept inside the ISR itself
	//but i wanted to demonstrate how can we read the data saved
	//by the isr properly and when data is continuously sent by other device.


		if(new_data)
		{
			cli();
			new_data=0;//another mechanism would be that i would first
					//search the string for the starting character of
					//my string then from that address i will increment the
					//address and see if my required string is present if yes
					//then i can take my action

					//but since strstr() function does the same thing
					//it saves my time and effort

			//call interfacing part
			if(check_response("RING"))
			{
				cmd(0x01);
				if(check_response("9437409515"))
				{
					//action to take when a call from above number is received
					lcd_print("call received from my phone");
				}

				else
				{
					//action to take when a call from any other number is received
					lcd_print("Someone else calling");
				}

			}

			//message interfacing part

			else if(check_response("CMT")) //here we can take another condition
			{						//to also confirm the number to consider the sms
				//action to take when a msg with the below text is received
				if(check_response("ledon"))
					led_on();

				if(check_response("ledoff"))
					led_off();
			}

			//For more commands read the GSM Commands.txt file

			else read_buffer();
			//if none of the above conditions are satisfied then
			//display data in buffer

		}

		sei();


	}

	return 0;
}




ISR(USART_RX_vect)
{

	new_data=1;//when this variable is set to 1 then i can check in the main loop
	buffer_index =0; //that there is some new data in buffer to decode.
	timer = 0;

	gsm_buffer[buffer_index] = UDR0;

	//this algorithm makes sure that it stops waiting for characters if there is
	//more than 1s delay between characters as according to our clock frequency
	//and timer prescaler value set a value of 45 will result in approx delay of 1s.

	//for a known baudrate and a known maximum response time we can decrease or increase
	//the timeout period accordingly

	for(buffer_index=1;buffer_index<100 && timer < 45;buffer_index++)
	{

		reset_timer();

		while(!(UCSR0A & (1<<RXC0)) && timer < 45)
		{
			if(TIFR0 & (1<<TOV0))
			{
				timer++;
				TIFR0 |= (1<<TOV0);
			}
		}

		if(timer < 45) //so that no garbage is written
		gsm_buffer[buffer_index] = UDR0;
	}
	gsm_buffer[--buffer_index] = '\0'; //because even if timer value is more than 45 after each cycle buffer_index increments
										//so buffer_index will increment by 2 values more than received number of characters
}
