#include "Arduino.h"
#define TEST_CS1694
//#define TEST_MC14499

#ifdef TEST_CS1694
#include "CS1694.h"
#include "myCS1694.h"
#endif

#ifdef TEST_MC14499
#include "MC14499.h"
#endif

#ifndef DEFAULT_BAUDRATE
	#define DEFAULT_BAUDRATE 115200
#endif

void setup() {
	Serial.begin(DEFAULT_BAUDRATE);

#ifdef TEST_CS1694
	setup_CS1694();

	displayMode(MODE_7_GRIDS_10_SEGS);
	displayControl(DISPLAY_ON, 0x04);
	displayDigit(3, 0);
	displayDigit(5, 1);
	displayChar('a', 2);
	displayChar('-', 3);
	displayChar('b', 4);
	displayChar('=', 5);
	displayChar('?', 6);

	setSegment(circle[0], 1);
	setSegment(circle[2], 1);
	setSegment(circle[4], 1);
	setSegment(circle[6], 1);
	setSegment(logos[LOGO_Colon_A], 1);

	sendSegments();
#endif


#ifdef TEST_MC14499
	setup_MC14499();
	digitalWrite(O_ENABLE, HIGH);

	digitBuffer.d1 = 1;
	digitBuffer.d2 = 2;
	digitBuffer.d3 = 3;
	digitBuffer.d4 = 4;
	Serial.print("test buffer : 1234 => "); Serial.println(digitBuffer.buffer, HEX);

	delay(200);
	sendDigits();

	digitBuffer.buffer= 0;
#endif

	Serial.println("setup ok");
}

byte convertedKeys = 0b00000000;

byte convertKeys() {
	return 0b11100000
		| ( (keys[1] & 0b000010) << 3 )
		| ( (keys[1] & 0b000100) << 1 )
		| ( (keys[0] & 0b100000) >> 3 )
		| ( (keys[1] & 0b010000) >> 3 )
		| ( (keys[1] & 0b100000) >> 5 );
}

byte atoh(byte c) {
	if (c >='0' && c <= '9') {
		return c - '0';
	} else if(c >='A' && c <= 'F') {
		return c - 'A' + 10;
	} else if(c >='a' && c <= 'f') {
		return c - 'a' + 10;
	} else {
		return 0;
	}
}

byte inputBuffer[29];
byte currentInput = 0;
byte petal = 0;

void rotate() {
	delay(100);
	setSegment(circle[petal], 0);
	petal = (petal + 1) & 7;
	setSegment(circle[petal], 1);
	sendSegments();
}

void loop() {
#ifdef TEST_CS1694
	readKeys();
	byte k = convertKeys();
	if (k != convertedKeys) {
		convertedKeys = k;
		Serial.println(convertedKeys, BIN);
	}

	if (convertedKeys & 0x10) {
		rotate();
	}
	while(Serial.available()) {
		int c = Serial.read();
		if (c < 0) {
			break;
		}

		if (c == '+') {
			c = Serial.read();
			byte l = (c - '0') & 7;
			displayControl(DISPLAY_ON, l);
			Serial.print("brightness ");
			Serial.println(l);
			c = Serial.read(); // skip CR

		} else if (c == '\n' || c == '\r') {
			for(byte i=currentInput; i<28; i++) {
				inputBuffer[i]=0; // Pad end of buffer with 0
			}
			for(byte i=0; i<14; i++) {
				segmentRam[i] = (atoh(inputBuffer[2*i]) << 4) | atoh(inputBuffer[2*i + 1]);
			}
			Serial.print("Read  : ");
			for (byte i=0; i<14; i++) {
				Serial.print(segmentRam[i], HEX);
				Serial.print(' ');
			}
			Serial.println();

			sendSegments();

			currentInput = 0;
		} else if (currentInput < 28) {
			inputBuffer[currentInput++] = c;
		}
	}
#endif


#ifdef TEST_MC14499
	delay(2000);
	digitBuffer.buffer+=1234;
	Serial.println(digitBuffer.buffer, HEX);
	sendDigits();
#endif
}
