#include <avr/io.h>
#include "define.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

volatile uint8_t isPlaying = 0;
volatile uint8_t countTimer0 = 0;
// const uint8_t TOP[17] = {219, 207, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116, 109, 103, 97, 92, 86};
extern const uint8_t TOP[108];

volatile char recv_buffer[RX_BUFFER_SIZE+1];
volatile uint8_t line[RX_BUFFER_SIZE+1];
volatile uint8_t rx_curr_index = 0;	
volatile char u_data;

volatile uint8_t ready_to_check_ipd = 0;  

volatile uint8_t isRecvResponse = 0x00; // bit 0: OK, bit 1: ERROR, bit 2: FAIL, bit 3: CONNECTED, bit 4: CLOSED

uint8_t curr_cmd;

inline void UART_Init() {
	
	// set baudrate: Tan so doc/ghi ben gui va ben nhan
	UBRRH = 0x00;
	UBRRL = _UBRR;
	
	// Set enable mode hoat dong nhan va gui, ngat nhan
	UCSRB = (1<<RXEN) | (1<<TXEN) | (1<<RXCIE);
	
	// Set dinh dang du lieu gui : 1 bit Start, 8 bit Data, 0 Bit parity, 1 bit Stop 
	UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);
}

inline void UART_Transmit(unsigned char byte) {
	while (!(UCSRA & DATA_REGISTER_EMPTY)) {};
	UDR = byte;
}

inline void send_cmd(const char* CMD) {
	uint8_t length = strlen(CMD);
	uint8_t i = 0;
	while (i < length) {
		UART_Transmit(CMD[i]);
		i++;
	}
	
	UART_Transmit('\r');
	UART_Transmit('\n');
	wait_for_response();
}

inline void connect2Server() {
	char TCP_ESTB_cmd[40];
	sprintf(TCP_ESTB_cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d", IP_SERVER, PORT);
	
	curr_cmd = CIPSTART;
	send_cmd(TCP_ESTB_cmd);
}

inline void joinAP() {
	char JAP_cmd[30];
	sprintf(JAP_cmd, "AT+CWJAP_DEF=\"%s\",\"%s\"", SSID, PASSWORD);
	curr_cmd = CWJAP;
	send_cmd(JAP_cmd);
}

inline void Wifi_Init() {
	// Switches echo off
	curr_cmd = ECHO_OFF;
	send_cmd("AT");
	
	curr_cmd = CWMODE;
	send_cmd("AT+CWMODE_DEF=1");

	joinAP();
	
	// Set single connection
	curr_cmd = CIPMUX;
	send_cmd("AT+CIPMUX=0");

	connect2Server();
}

inline void Sound_Init() {
	DDRB = (1<<PB1) | (1<<PB2);

	TCCR0 = (1<<CS02) | (1<<CS00);
	TCNT0 = _TCNTO;
	TIMSK = (1<<TOIE0); 			
}

inline void pause() {
	isPlaying = 0;
	TCCR1A = 0x00;
	TCCR1B = 0x00;

	PORTC = 0x00;
	PORTB &= ~LED_G;
}

inline void startTiming() {
	// start timing in 200ms
	TCNT0 = _TCNTO;
	countTimer0 = 0;
	isPlaying = 1;
}

inline void play(uint8_t noteCode) {
	TCNT1 = 0;
	TCCR1A = (1<<COM1A0);	// Toggle on Compare Match
	TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10); // CTC, clk/64

	OCR1A = TOP[noteCode];
	startTiming();
}

inline void wait_for_response() {
	while (!isRecvResponse) {}
		
	if (isRecvResponse & OK) { // OK for all
		if (curr_cmd & CIPSTART) {
			isRecvResponse |= (1<<CONNECTED);
			PORTC |= (1<<PC4);
		}

		PORTC |= (1<<PC5);
	}
		
	if (isRecvResponse & ERROR) { // ERROR for AT+CIPSTART
		PORTC |= (1<<PC3);
	}
		
	if (isRecvResponse & FAIL) { // FAIL for AT+CWJAP 
		PORTC |= (1<<PC4);
	}
	
	isRecvResponse = 0x00;
}

//msg: Xn\r\n or Xn#\r\n 
inline uint8_t getNoteCode(const char* msg) {
	uint8_t noteId = -1;
	uint8_t octave = atoi(msg+1);

	switch (msg[0]) {
		case 'C': 
			noteId = octave*12;
			if (msg[2] == '#') noteId++;
			PORTC = LED_C;
			break;
			
		case 'D':
			noteId = octave*12 + 2;
			if (msg[2] == '#') noteId++;
			PORTC = LED_D;
			break;
			
		case 'E':
			noteId = octave*12 + 4;
			PORTC = LED_E;
			break;
			
		case 'F':
			noteId = octave*12 + 5;
			if (msg[2] == '#') noteId++;
			PORTC = LED_F;
			break;
			
		case 'G':
			noteId = octave*12 + 7;
			if (msg[2] == '#') noteId++;
			PORTB |= LED_G;
			break;
			
		case 'A':
			noteId = octave*12 + 9;
			if (msg[2] == '#') noteId++;
			PORTC = LED_A;
			break;
			
		case 'B':
			noteId = octave*12 + 11;
			PORTC = LED_B;
			break;
	}
	
	return noteId;
}

inline void check_note() {
	ready_to_check_ipd = 0;
	uint8_t msg[10];
	
	// Get message
	uint8_t i = 0;
	
	while (line[i] != ':') i++;
	strcpy(msg, line + i + 1);
	
	uint8_t noteCode = getNoteCode(msg);
	
	if (noteCode != -1) {
		play(noteCode);
	}
}

inline void check_wifi_init_res() {
	uint8_t line_length = strlen(line);
	uint8_t i = 0;
	while (i < line_length) {
		// Wifi Init Response
		if (line[i] == 'O' && line[i+1] == 'K') {
			isRecvResponse = OK;
			return;
		}
	
		if (line[i] == 'E' && line[i+1] == 'R' && line[i+2] == 'R' && 
			line[i+3] == 'O' && line[i+4] == 'R') 
		{
			isRecvResponse = ERROR;
			return;
		}
	
		if (line[i] == 'F' && line[i+1] == 'A' && line[i+2] == 'I' && line[i+3] == 'L') {
			isRecvResponse |= FAIL;
			return;
		}
		
		i++;
	}
}

ISR(USART_RXC_vect) {	
	// Dam bao du lieu dung, thi can doc data ra truoc
	u_data = UDR;
	recv_buffer[rx_curr_index++] = u_data;
	recv_buffer[rx_curr_index] = '\0';
	
	if (u_data == '\n') {
		strcpy(line, recv_buffer);
		
		if (line[0] == '+' && line[1] == 'I' && line[2] == 'P' && line[3] == 'D') ready_to_check_ipd = 1;
		else if (line[0] == 'C' && line[1] == 'L' && line[2] == 'O' && line[3] == 'S' && line[4] == 'E' && line[5] == 'D') isRecvResponse |= CLOSED;
		else check_wifi_init_res();

		rx_curr_index = 0;
	} 

	if (rx_curr_index > RX_BUFFER_SIZE) rx_curr_index = 0;
}

ISR(TIMER0_OVF_vect) {
	if (isPlaying) {
		TCNT0 = _TCNTO;
		countTimer0++;
		
		if (countTimer0 == LOOP_TIMER0) {
			pause();
		}
	}
}

int main(void) {
	Sound_Init();	
	
	UART_Init();
	
	sei();
	
	DDRC = 0x3F;
	
	Wifi_Init();
	
	while (1) {
		if (ready_to_check_ipd) {
			check_note();
		}

		if (isRecvResponse & FAIL) {
			_delay_ms(2000);
			joinAP();
		}

		if (isRecvResponse & CLOSED) {
			_delay_ms(2000);
			connect2Server();
		}
	}
	
	return 0;   
}

