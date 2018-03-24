/*
 * DA3_Task1.c
 *
 * Created: 3/18/2018 1:54:58 PM
 * Author : Damian Cisneros
 * Description : This program monitors temperature using an LM34 sensor. It
 *				 reads the temperature every 1s and displays it on a serial
 *				 terminal. Using 8Mhz clock
 */ 

#define BAUD 9600
#define F_CPU 8000000UL

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
//#include <stdint.h> //needed for uint8_t
#include <util/delay.h>

static int put_char(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(put_char, NULL, _FDEV_SETUP_WRITE);

void init_ADC();
void init_USART();
void USART_tx_string(char*);
//void writeData(unsigned char);


int main(void)
{
	float ADCvalue; //holds converted ADC value
	char c[9]; //holds converted value in string

	stdout = &mystdout; //set the output stream

	init_USART();  
	init_ADC();

	while(1){
		ADCSRA |= (1 << ADSC); //start the conversion. while in free running mode it will
		while((ADCSRA&(1 << ADIF))==0); //check if conversion done
		ADCSRA |= (1 << ADIF); //reset flag
		ADCvalue = ADC & 0x03ff; //grab all 10 bits from ADC
		ADCvalue = ((ADCvalue * 5)/1024)*100; //convert to degrees Fahrenheit
		
		//itoa(ADCvalue, c, 10); //convert int to string
		
		dtostrf(ADCvalue,3,1,c); //convert double to string
		printf("Temperature: ");
		USART_tx_string(c); //print value to terminal
		printf(" F\r\n");
		_delay_ms(1000); //wait 1s
	}
}

void init_USART(){
	unsigned int BAUDrate;

	//set BAUD rate: UBRR = [F_CPU/(16*BAUD)]-1
	BAUDrate = ((F_CPU/16)/BAUD) - 1;
	UBRR0H = (unsigned char) (BAUDrate >> 8); //shift top 8 bits into UBRR0H
	UBRR0L = (unsigned char) BAUDrate; //shift rest of 8 bits into UBRR0L
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0); //enable receiver and trasmitter
	// UCSR0B |= (1 << RXCIE0); //enable receiver interrupt
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); //set data frame: 8 bit, 1 stop
}

void init_ADC(){
	ADMUX = 0; //use ADC0
	ADMUX |= (1 << REFS0); //use AVcc as the reference (5V)
	//ADMUX |= (1 << ADLAR); //set to right adjust for 8-bit ADC

	//ADCSRA |= (1 << ADIE); //ADC interrupt enable
	ADCSRA |= (1 << ADEN); //enable ADC
	
	//set pre-scale to 128 for input frequency
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	ADCSRB = 0; //free running mode
}

void USART_tx_string(char* data){
	while((*data!='\0')){ //print until null
		while(!(UCSR0A &(1<<UDRE0))); //check if transmit buffer is ready for new data
		UDR0=*data; //print char at current pointer
		data++; //iterate char pointer
	}
}

static int put_char(char c, FILE *stream)
{
	while(!(UCSR0A &(1<<UDRE0))); // wait for UDR to be clear
	UDR0 = c;    //send the character
	return 0;
}