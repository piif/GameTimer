#include "Arduino.h"

#include "CS1694.h"
#include "myCS1694.h"

#ifndef DEFAULT_BAUDRATE
	#define DEFAULT_BAUDRATE 115200
#endif

#define BIP_PIN 6

// TODO : read/write this value from eeprom
int durationIndex = 1;

int durations[] = {
	5, 10, 15, 20, 30, 45,
	60, 120, 180, 240, 300, 360, 420, 480, 540, 600,
	900, 1200, 1500, 1800
};
#define nbDurations (sizeof(durations)/sizeof(int))

int preAlarm(int duration) {
	if (duration > 600) {
		return 60;
	}
	if (duration > 60) {
		return 10;
	}
	return 2;
}

int displayTime(int seconds) {
	char digits[4] = { ' ', };
	setSegment(logos[LOGO_Colon_B], seconds >= 60);

	digits[3] = '0' + seconds % 10;
	if (seconds >= 10) {
		digits[2] = '0' + (seconds / 10) % 6;
	}
	if (seconds >= 60) {
		int minutes = seconds / 60;
		digits[1] = '0' + minutes % 10;
		if (minutes >= 10) {
			digits[0] = '0' + (minutes / 10);
		}
	}

	displayChar(digits[0], 3);
	displayChar(digits[1], 4);
	displayChar(digits[2], 5);
	displayChar(digits[3], 6);

	sendSegments();
}

byte convertedKeys = 0;

byte convertKeys() {
	return 0b11100000
		| ( (keys[1] & 0b000010) << 3 )
		| ( (keys[1] & 0b000100) << 1 )
		| ( (keys[0] & 0b100000) >> 3 )
		| ( (keys[1] & 0b010000) >> 3 )
		| ( (keys[1] & 0b100000) >> 5 );
}

#define KEY_PLUS  ((convertedKeys & 0x18) == 0x08)
#define KEY_MINUS ((convertedKeys & 0x18) == 0x10)
#define KEY_TWICE ((convertedKeys & 0x18) == 0x18)
#define KEY_ANY   ((convertedKeys & 0x18) != 0x00)

// return '+', '-', '2', or 0
char keyPressed() {
	static char currentKey = 0;

	readKeys();
	byte k = convertKeys() & 0x18;
	if (k != convertedKeys) {
		convertedKeys = k;
		Serial.println(convertedKeys, HEX);
		if (convertedKeys == 0) {
			char r = currentKey;
			currentKey = 0;
			return r;
		} else if (convertedKeys == 0x18) {
			currentKey = '2';
			Serial.println("-> 2");
		} else if (currentKey != '2') {
			if (convertedKeys == 0x10) {
				currentKey = '-';
				Serial.println("-> -");
			} else if(convertedKeys == 0x08) {
				currentKey = '+';
				Serial.println("-> +");
			}
		}
	}
	return 0;
}

void selectDuration() {
	// display "SET:"
	displayChar('S', 0);
	displayChar('E', 1);
	displayChar('T', 2);
	setSegment(logos[LOGO_Colon_A], 1);
	displayTime(durations[durationIndex]);

	for(;;) {
		switch(keyPressed()) {
			case '2':
				Serial.println('2');

				displayChar(' ', 0);
				displayChar(' ', 1);
				displayChar(' ', 2);
				setSegment(logos[LOGO_Colon_A], 0);
			return;
			case '-':
				Serial.println('-');
				durationIndex = (durationIndex + nbDurations - 1) % nbDurations;
				displayTime(durations[durationIndex]);
			break;
			case '+':
				Serial.println('+');
				durationIndex = (durationIndex + 1) % nbDurations;
				displayTime(durations[durationIndex]);
			break;
		}
	}
}

unsigned long counterEnd; // uptime when counting will expire
int count; // current counting value
int alarm1, alarm2; // count where pre alarm bip must be played

void initCounter() {
	count = durations[durationIndex];
	counterEnd = millis() + 1000L * (1 + durations[durationIndex]);
	displayTime(count);
	alarm1 = preAlarm(count);
	alarm2 = alarm1 / 2;
}

void setup() {
	Serial.begin(DEFAULT_BAUDRATE);

	setup_CS1694();

	displayMode(MODE_7_GRIDS_10_SEGS);
	displayControl(DISPLAY_ON, 0x07);
	sendSegments();

	pinMode(BIP_PIN, OUTPUT);

	Serial.println("setup ok");

	selectDuration();
	initCounter();
}

//byte petal = 0;
//
//void rotate() {
//	delay(100);
//	setSegment(circle[petal], 0);
//	petal = (petal + 1) & 7;
//	setSegment(circle[petal], 1);
//	sendSegments();
//}

void loop() {
	switch(keyPressed()) {
		case '2':
			selectDuration();
			initCounter();
		break;
		case '-':
		case '+':
			initCounter();
		break;
	}

	int newCount = (counterEnd - millis()) / 1000;
	if (newCount >= 0 && newCount != count) {
		count = newCount;
		if (count == alarm1 || count == alarm2) {
			tone(BIP_PIN, 880, 50);
		} else if (count == 0) {
			tone(BIP_PIN, 220, 500);
		}
		displayTime(count);
	}

//	if (convertedKeys & 0x10) {
//		rotate();
//	}
//	if (convertedKeys & 0x08) {
//		tone(BIP_PIN, 880, 50);
//		delay(1000);
//	}
//	if (convertedKeys & 0x04) {
//		tone(BIP_PIN, 220, 500);
//		delay(1000);
//	}

}
