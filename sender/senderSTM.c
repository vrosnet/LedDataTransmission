/*
============================================================================
Name :        senderSTM.h
Author :      ChrisMicro
Version :
Copyright :   GPL license 3


Description : sender state machine
============================================================================
*/

#include <stdio.h>
#include "mc_io.h"
#include "senderSTM.h"

#define SYSTEMOUTCHAR(c) //putchar(c)

/***************************************************************************************

	bitStates sendBit_S(enum bitVal bit)

    send one bit state machine

	//enum bitStates { BITREADY=0, LOW_S2, HIGH_S2,HIGH_S3,HIGH_S4 };

    input: bit value

***************************************************************************************/

enum bitStates sendBit_S(uint16_t bit)
{
	static enum bitStates state=BITREADY;

	switch(state)
	{
		case BITREADY:{
			SYSTEMOUTCHAR(' ');
			ledOff();
			if(bit==0)	    state=LOW_S2;
			else 			state=HIGH_S2;
		}break;

		// low bit states
		case LOW_S2:{
			ledOn();		state=BITREADY; // final state go back to ready
		}break;
		// =================================

		// =================================
		// high bit states
		case HIGH_S2:{
			ledOff();		state=HIGH_S3;
		}break;
		case HIGH_S3:{
			ledOn();		state=HIGH_S4;
		}break;
		case HIGH_S4:{
			ledOn();		state=BITREADY; // final state go back to ready
		}break;

		// =================================
	}
	return state;
}
/***************************************************************************************

	byteSenderStates sendByte_S(uint16_t byte)

    State machine for sending a byte.

    input: byte including startbit

***************************************************************************************/
#define NUMBEROFBITS 9 // start bit + data bits
#define STARTBIT 0x100 // first bit is start bit

enum byteSenderStates { READYFORNEXTBYTE,STARTSENDING,SENDING };

enum byteSenderStates sendByte_S(uint16_t byte)
{
	static enum byteSenderStates state=READYFORNEXTBYTE;
	static uint8_t bitCounter;
	static uint16_t dat;

	switch(state)
	{
		case READYFORNEXTBYTE:{
			bitCounter=8;
			dat=byte;

			// start of first bit
			// the first bit should be the start bit
			// the start bit is set in dat before calling this routine ( dat|=0x100 )
			sendBit_S(dat&(1<<(NUMBEROFBITS-1)));
			state=STARTSENDING;

		}break;
		case STARTSENDING:{
			// go on with sending bit
			if(sendBit_S(dat&(1<<(NUMBEROFBITS-1)))==BITREADY)
			{
				dat=dat<<1; // first bit is sent
				state=SENDING;
			}
		}break;

		case SENDING:{
			if(sendBit_S(dat&(1<<(NUMBEROFBITS-1)))==BITREADY)
			{
				dat=dat<<1;
				bitCounter--;
				if(bitCounter==0)
				{
					state=READYFORNEXTBYTE; // leave sender state
					SYSTEMOUTCHAR('\n');
				}
			}
		}break;
	}

	return state;
}
/***************************************************************************************

	SenderStates sendFrame_S(uint8_t *data, uint8_t dataLen)

    Stae machine for sending a frame.

    input: data array, data length

***************************************************************************************/
//enum SenderStates { FRAMEREADY,FRAMESTART,PREAMPLE,SENDDATA };

#define PREAMPLELENGTH 5 // length of preample in bytes

enum SenderStates sendFrame_S(uint8_t *data, uint8_t dataLen)
{
	static enum SenderStates state=FRAMEREADY;
	static uint8_t *dataPointer;
	static uint8_t datLen;
	static uint8_t preampleCounter;

	switch(state)
	{
		case FRAMEREADY:{
			preampleCounter=PREAMPLELENGTH;
			dataPointer=data;
			datLen=dataLen;

			sendByte_S(0); // first bit of preample
			state=FRAMESTART;
		}break;

		case FRAMESTART:{
			sendByte_S(0); // go on with sending first bit
			state=PREAMPLE;
		}break;

		case PREAMPLE:{
			if(sendByte_S(0)==READYFORNEXTBYTE)
			{
				preampleCounter--;
				if(preampleCounter==0)	state=SENDDATA;
			}
		}break;

		case SENDDATA :{
			if(sendByte_S(*dataPointer+STARTBIT)==READYFORNEXTBYTE)
			{
				dataPointer++;
				datLen--;
				if(datLen==0) state=FRAMEREADY;
			}
		}break;
	}

	return state;
}


