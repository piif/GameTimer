#include "Arduino.h"
#include "CS1694.h"

#define CLOCK_PIN  8
#define STROBE_PIN 9
#define DATA_PIN  10
#define IR_PIN    11

byte segmentRam[14] = { 42, };
//	0xFF, 0x0F,
//	0x11, 0x01,
//	0x23, 0x02,
//	0x47, 0x04,
//	0x8F, 0x08,
//	0x18, 0x0F,
//	0x2C, 0xFF,
//};

byte keys[5];

void sendByte(byte data) {
	// dosn't work because (i suppose ...) shiftOut is written to write data on falling edge, but CS1694 waits data on rising edge
//	shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, data);
	for(byte i = 8; i; i--) {
		digitalWrite(CLOCK_PIN, LOW);
		digitalWrite(DATA_PIN, data & 0x01);
		digitalWrite(CLOCK_PIN, HIGH);
		data >>= 1;
	}
}

byte readByte() {
	return shiftIn(DATA_PIN, CLOCK_PIN, LSBFIRST);
//	byte data = 0;
//	for(byte m = 1; m; m <<= 1) {
//		digitalWrite(CLOCK_PIN, HIGH);
//		byte b = digitalRead(DATA_PIN);
//		if (b) {
//			data |= m;
//		}
//		digitalWrite(CLOCK_PIN, LOW);
//	}
//	return data;
}

void sendCommand(byte command) {
	digitalWrite(STROBE_PIN, HIGH);
	digitalWrite(CLOCK_PIN, HIGH);
	digitalWrite(STROBE_PIN, LOW);
	sendByte(command);
	digitalWrite(STROBE_PIN, HIGH);
}

void sendWriteCommand(byte len, byte data[]) {
	digitalWrite(STROBE_PIN, HIGH);
	digitalWrite(CLOCK_PIN, HIGH);

//	digitalWrite(STROBE_PIN, LOW);
//	sendByte(OPERATION_MODE_COMMAND | OPERATION_WRITE | ADDRESS_AUTO_PLUS_1 | NORMAL_MODE);
//	digitalWrite(STROBE_PIN, HIGH);
//
//	digitalWrite(STROBE_PIN, LOW);
//	byte address = 0;
//	while(len--) {
//		sendByte(SET_ADDRESS_COMMAND | address);
////		delayMicroseconds(1);
//		sendByte(data[address]);
//		address++;
//	}
//	digitalWrite(STROBE_PIN, HIGH);

	Serial.print("sendWriteCommand : ");
	for (byte i=0; i<len; i++) {
		Serial.print(data[i], HEX);
		Serial.print(' ');
	}
	Serial.println();

	digitalWrite(STROBE_PIN, LOW);
	sendByte(SET_ADDRESS_COMMAND | 0);
	digitalWrite(STROBE_PIN, HIGH);

	digitalWrite(STROBE_PIN, LOW);
	sendByte(OPERATION_MODE_COMMAND | OPERATION_WRITE | ADDRESS_AUTO_PLUS_1 | NORMAL_MODE);
	while(len--) {
		sendByte(*data);
		data++;
	}
	digitalWrite(STROBE_PIN, HIGH);
}

void sendReadCommand(byte len, byte data[]) {
	digitalWrite(STROBE_PIN, HIGH);
	digitalWrite(CLOCK_PIN, HIGH);
	digitalWrite(STROBE_PIN, LOW);
	sendByte(OPERATION_MODE_COMMAND | OPERATION_READ | ADDRESS_FIXED | NORMAL_MODE);

	pinMode(DATA_PIN, INPUT);
	while(len--) {
		*data= readByte();
		data++;
	}
	digitalWrite(STROBE_PIN, HIGH);
	pinMode(DATA_PIN, OUTPUT);
}

void readKeys() {
	sendReadCommand(5, keys);
}

void sendSegments() {
//	setAddress(0);
	sendWriteCommand(14, segmentRam);
}

void setup_CS1694() {
	pinMode(STROBE_PIN, OUTPUT);
	pinMode(CLOCK_PIN,  OUTPUT);
	pinMode(DATA_PIN,   OUTPUT);
	pinMode(IR_PIN,     INPUT);
}
