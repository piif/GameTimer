#include "Arduino.h"

#define O_ENABLE 4
#define O_CLOCK  3
#define O_DATA   2

extern union _digitBuffer {
	word buffer;
	struct {
		byte d4:4;
		byte d3:4;
		byte d2:4;
		byte d1:4;
	};
} digitBuffer;

void sendDigits();

void setup_MC14499() {
	pinMode(O_ENABLE, OUTPUT);
	pinMode(O_CLOCK,  OUTPUT);
	pinMode(O_DATA,   OUTPUT);
}
