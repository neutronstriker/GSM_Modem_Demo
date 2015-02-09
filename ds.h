#ifndef _DATA_STRUCTURES_
#define _DATA_STRUCTURES_ 1

//Important : Please be carefull while using stack as multiple functions are using the same stack so use it carefully.
//This cannot be used as a alternative to the default stack of the processor, because multiple programs can
//modify the TOP and BOTTOM value. But it can be used in functions where data processing involves use of
//stack but it completes its all operation along with retrieving data from the stack without interruption.

//It means you can call a function which will load data into the stack and get back all the data within the same
//call. Or else make sure that no other stack based function is used.
/*
Author : N.Srinivas a.k.a NeutroN StrikeR
Date:19-06-2014
email:striker.dbz[at]hotmail.com

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

#define max 20
unsigned char stack[max]; //stack size
int top = -1;  //Initialize stack pointer

void push(unsigned char);
unsigned char pop();

void push(unsigned char element)
{
	if(top == (max-1))
	{
		lcd_print("Stack is full" ,0x80);
		return;
	}
	top = top+1;
	stack[top] = element;
}

unsigned char pop()
{
	if(top == -1)
	{
		lcd_print("Stack is empty", 0x80);
		return 0;
	}
	return(stack[top--]);

}

//In future we are going to build queue,circular queue(fifo buffer), linkedlist here

#endif
