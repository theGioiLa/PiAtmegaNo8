#define F_CPU 7372800UL
		
// UART Config
#define BAUD 115200UL
#define _UBRR (F_CPU/16/BAUD - 1)

// OCR1A formal
#define PRESCALER F_CPU/64
#define TOP(x) (PRESCALER/2/x - 1)

// Timer0 Loop count
#define LOOP_TIMER0 10
#define _TCNTO (256 - 0.2*F_CPU/1024/LOOP_TIMER0)

// UART Data Flag bit
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<PE)
#define DATA_OVERRUN (1<<DOR)

// Wifi Config
#define IP_SERVER	"192.168.137.1"
#define PORT		8124
#define SSID		"UART"
#define PASSWORD	"12345678"


#define RX_BUFFER_SIZE 64

// Response message
#define OK (1<<0)
#define ERROR (1<<1)
#define FAIL (1<<2)
#define CONNECTED (1<<3)
#define CLOSED (1<<4) // server closed

// Current AT Command
#define ECHO_OFF (1<<0)
#define CWMODE (1<<1)
#define CWJAP (1<<2)
#define CIPMUX (1<<3)
#define CIPSTART (1<<4)


// LED Note
#define LED_C (1<<PC2)
#define LED_D (1<<PC3)
#define LED_E (1<<PC4)
#define LED_F (1<<PC5)
#define LED_G (1<<PB2)
#define LED_A (1<<PC0)
#define LED_B (1<<PC1)

const uint8_t TOP[108] = {
    3522, 3324, 3137, 2961, 2795, 2638, 2490, 2350, 2218, 2094, 1976, 1865, //  Octave 0
    1760, 1661, 1568, 1480, 1397, 1318, 1244, 1175, 1109, 1046, 987, 932,   //  Octave 1
    880, 830, 784, 740, 698, 659, 622, 587, 554, 523, 493, 466,             //  Octave 2
    439, 415, 391, 369, 348, 329, 310, 293, 276, 261, 246, 232,             //  Octave 3
    219, 207, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116,             //  Octave 4
    109, 103, 97, 92, 86, 81, 77, 72, 68, 64, 61, 57,                       //  Octave 5
    54, 51, 48, 45, 43, 40, 38, 36, 34, 32, 30, 28,                         //  Octave 6
    27, 25, 24, 22, 21, 20, 18, 17, 16, 15, 14, 14,                         //  Octave 7
    13, 12, 11, 11, 10, 9, 9, 8, 8, 7, 7, 6                                 //  Octave 8
};