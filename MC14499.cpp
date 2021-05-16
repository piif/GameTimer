#include "Arduino.h"

#define O_ENABLE 4
#define O_CLOCK  3
#define O_DATA   2

#ifndef DEFAULT_BAUDRATE
	#define DEFAULT_BAUDRATE 115200
#endif

union _digitBuffer {
	word buffer;
	struct {
		byte d4:4;
		byte d3:4;
		byte d2:4;
		byte d1:4;
	};
} digitBuffer;

void sendDigits() {
	word buffer = digitBuffer.buffer;

	digitalWrite(O_CLOCK, HIGH);
	digitalWrite(O_ENABLE, LOW);
	for (word mask = 0x8000; mask; mask >>= 1) {
		digitalWrite(O_CLOCK, HIGH);
		digitalWrite(O_DATA, digitBuffer.buffer & mask ? HIGH : LOW);
		digitalWrite(O_CLOCK, LOW);
	}
	digitalWrite(O_ENABLE, HIGH);
}

void setup_MC14499() {
	pinMode(O_ENABLE, OUTPUT);
	pinMode(O_CLOCK,  OUTPUT);
	pinMode(O_DATA,   OUTPUT);
}
