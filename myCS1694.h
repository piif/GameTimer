#include "Arduino.h"
#include "CS1694.h"

// address for each segment
byte digitSegments[] = {
	0x0, 0x2, 0x4, 0x6, 0x8, 0xA, 0xC
};

// bit position for each digit
byte digitBits[] = {
	0, 1, 7, 2, 3, 4, 5
};

typedef struct _segmentAddress {
	byte address;
	byte mask;
} segmentAddress;

#define LOGO_Colon_A 0
#define LOGO_Colon_B 1
#define LOGO_DVD     2
#define LOGO_VCD     3
#define LOGO_MP3     4
#define LOGO_Play    5
#define LOGO_Pause   6
#define LOGO_PBC     7
#define LOGO_Loop    8
#define LOGO_DTS     9
#define LOGO_DDD    10

segmentAddress logos[] = {
	{ 0x2, 0x40 }, // colon A
	{ 0x4, 0x40 }, // colon B
	{ 0x1, 0x02 }, // DVD
	{ 0x3, 0x02 }, // VCD
	{ 0x5, 0x02 }, // MP3
	{ 0x9, 0x02 }, // Play
	{ 0xB, 0x02 }, // Pause
	{ 0x7, 0x02 }, // PBC
	{ 0x0, 0x40 }, // Loop
	{ 0x8, 0x40 }, // DTS
	{ 0x6, 0x40 }  // DDD
};

segmentAddress circle[] = {
	{ 0xD, 0x02 }, // top left left
	{ 0x1, 0x01 }, // top top left
	{ 0x3, 0x01 }, // top top right
	{ 0xB, 0x01 }, // top right right
	{ 0xD, 0x01 }, // bottom right right
	{ 0x7, 0x01 }, // bottom bottom right
	{ 0x5, 0x01 }, // bottom bottom left
	{ 0x9, 0x01 }  // bottom left left
};

#define LED7_CHARS "0123456789abcdefghijklmnopqrstuvwxyz ?-_='#"
const char *mapChars = LED7_CHARS;

// segment map for each of the characters listed in mapChars
byte mapSegments[] = {
	// tRrblLm
	0b01111110, // 0
	0b00110000, // 1
	0b01101101, // 2
	0b01111001, // 3
	0b00110011, // 4
	0b01011011, // 5
	0b01011111, // 6
	0b01110000, // 7
	0b01111111, // 8
	0b01111011, // 9
	0b01110111, // a
	0b00011111, // b
	0b01001110, // c
	0b00111101, // d
	0b01001111, // e
	0b01000111, // f
	0b01011110, // g
	0b00010111, // h
	0b00010000, // i
	0b00111000, // j
	0b00001111, // k
	0b00001110, // l
	0b01110110, // m
	0b00010101, // n
	0b00011101, // o
	0b01100111, // p
	0b01110011, // q
	0b00000101, // r
	0b01011011, // s
	0b00000111, // t
	0b00011100, // u
	0b00111110, // v
	0b00111111, // w
	0b00110111, // x
	0b00111011, // y
	0b01101101, // z
	0b00000000, // ' '
	0b01100101, // ?
	0b00000001, // -
	0b00001000, // _
	0b00001001, // =
	0b00000010, // '
	0b01100011, // #
};

byte getSegments(char c) {
	int i = 0;
	const char *ptr = mapChars;
	// no upper letters in the map => convert them to lower.
	if (c >= 'A' && c <= 'Z') {
		c = c - 'A' + 'a';
	}
	while (*ptr != '\0') {
		if (*ptr == c) {
			return mapSegments[i];
		}
		i++;
		ptr++;
	}
	return 0;
}

void setSegment(const byte address, const byte mask, const byte value) {
	if (value) {
		segmentRam[address] |= mask;
	} else {
		segmentRam[address] &= ~mask;
	}
}

void setSegment(const segmentAddress addressMask, const byte value) {
	setSegment(addressMask.address, addressMask.mask, value);
}

void displayChar(const char value, const byte position) {
	byte segments = getSegments(value);
	byte digitMask = 1 << digitBits[position];
	for (byte segmentMask = 0x40, segmentNum = 0; segmentMask; segmentMask >>= 1, segmentNum++) {
		if (segments & segmentMask) {
			segmentRam[digitSegments[segmentNum]] |= digitMask;
		} else {
			segmentRam[digitSegments[segmentNum]] &= ~digitMask;
		}
	}
}

void displayDigit(const byte digit, const byte position) {
	displayChar(digit + '0', position);
}
