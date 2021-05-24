#include "Arduino.h"
#include "notes.h"

#ifndef DEFAULT_BAUDRATE
	#define DEFAULT_BAUDRATE 115200
#endif

#define TEST_MICRO
//#define USE_COMPARATOR
//#define USE_ADC_WITH_LOOP
#define USE_ADC_WITH_INTERRUPT

//#define MAIN_CODE

#define writeReg16(reg, value) do { unsigned char sreg = SREG; cli(); reg=value; SREG = sreg; } while(0)
#define readReg16(var, reg) do { unsigned char sreg = SREG; cli(); var=reg; SREG = sreg; } while(0)

#define SPEAKER_PIN 11

#ifdef MAIN_CODE
#include "CS1694.h"
#include "myCS1694.h"

// TODO : read/write this value from eeprom
int durationIndex = 1;

static const int durations[] PROGMEM = {
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

	pinMode(SPEAKER_PIN, OUTPUT);

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
			tone(SPEAKER_PIN, 880, 50);
		} else if (count == 0) {
			tone(SPEAKER_PIN, 220, 500);
		}
		displayTime(count);
	}
}
#endif

#ifdef TEST_MICRO

#include "setInterval.h"

#ifdef USE_ADC_WITH_LOOP
#define INPUT_LEN 512
int inputs[INPUT_LEN] = { 0, };
#endif

#define BUTTON_NB 2
static const byte buttonPins[] = { 2, 3 };
static const byte buttonMsks[] = { 0x01, 0x02 };

void setupButtons() {
	for (byte i = 0; i < BUTTON_NB; i++) {
		pinMode(buttonPins[i], INPUT_PULLUP);
	}
}
byte buttonPressed = 0;

byte readButtonState() {
	byte newMask = 0;
	for (byte i = 0; i < BUTTON_NB; i++) {
		if (digitalRead(buttonPins[i])) {
			newMask &= ~buttonMsks[i];
		} else {
			newMask |= buttonMsks[i];
		}
	}
//	Serial.println(newMask);
	return newMask;
}

byte readButtons() {
	byte newMask = readButtonState();
	if (newMask != buttonPressed) {
		delay(50);
		newMask = readButtonState();
		if (newMask != buttonPressed) {
			byte result = 0;
			for (byte i = 0; i < BUTTON_NB; i++) {
				if ((newMask & buttonMsks[i]) && !(buttonPressed & buttonMsks[i])) {
					result |= buttonMsks[i];
				}
			}
			buttonPressed = newMask;
			return result;
		}
	}
	return 0;
}

#ifdef USE_COMPARATOR
volatile long comparatorRise = 0;
ISR(ANALOG_COMP_vect) {
	comparatorRise++;
}
#endif

void printPad(int value) {
	int abs = value < 0 ? -value : value;

	if (abs<10) {
		Serial.print("      ");
	} else if (abs < 100) {
		Serial.print("     ");
	} else if (abs < 1000) {
		Serial.print("    ");
	} else if (abs < 10000) {
		Serial.print("   ");
	} else {
		Serial.print("  ");
	}
	Serial.print(value < 0 ? '-' : ' ');
	Serial.print(abs);
}

#ifdef USE_ADC_WITH_INTERRUPT
// F_CPU = 16M => 8KHz = 125 timer ticks with 16 prescaler , or 250 with /8
// => TIMER2 + TIMER2_COMPA_vect

#define SAMPLES 128
volatile double vReal[SAMPLES];
double vImag[SAMPLES];
volatile int bufferIndex;

ISR(TIMER1_COMPA_vect) {
	if (bufferIndex == SAMPLES) {
//		bufferIndex=0;
		TIMSK1 &= ~(1 << OCIE1A);
	} else {
		vReal[bufferIndex++] = analogRead(A0);
	}
}

// from https://github.com/kosme/arduinoFFT/blob/master/Examples/FFT_01/FFT_01.ino
#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

const double samplingFrequency = 8000.0;

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / SAMPLES);
	break;
    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}

#endif

void setup() {
	Serial.begin(DEFAULT_BAUDRATE);

	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);

	// OC1A, for testing
	pinMode(9, OUTPUT);

	pinMode(A0, INPUT);
	setupButtons();

#ifdef USE_COMPARATOR
	/**
	 * analog comparator :
	 * ref voltage = AIN1 = D7
	 * input = AIN0 = D6
	 * ACME = 0 (= use AIN1) => ADCSRB |= 0x04
	 * ACSR = 00xx0011 = 0x03
	 *   ACD = 0
	 *   ACBG= 0
	 *   ACO = output (rd)
	 *   ACI = interrupt (rd)
	 *   ACIE = interrupt enable => 1  (/!\ + I=1 in Status register SREG |= 0x80)
	 *   ACIC = capture enable -> timer ??? => 0
	 *   ACIS1,0 : 11 for interrupt on rising
	 * DIDR1 => bit 0+1 = 1 to disable digital input => |=0x3
	 */
	ADCSRB |= 0x04;
	ACSR = 0x0B;
	DIDR1 |= 0x03;
#endif

#ifdef USE_ADC_WITH_INTERRUPT
	// prepare timer1 setup : OCRA = 250 , prescale 8 , no output, reset on OCRA
	TCCR1A = 0x40; // 0x00;
	TCCR1B = 0x0A;
//	TCCR1C = 0x80;
	TIMSK1 = 0;
	writeReg16(OCR1A, 249);
#endif
//	Serial.print("PRR = ");
//	Serial.println(PRR, HEX);
	Serial.println("setup ok");
	Serial.print("OCR1A = ");
	word ocr1a;
	readReg16(ocr1a, OCR1A);
	Serial.println(ocr1a);
}

void loop() {
	byte buttons = readButtons();
//	if (buttons != 0) {
//		Serial.print("=> ");
//		Serial.println(buttons);
//	}

	if (buttons & buttonMsks[1]) {
		Serial.println("tone ...");
		tone(SPEAKER_PIN, 440, 2000);
	}

	if (buttons & buttonMsks[0]) {
#ifdef USE_COMPARATOR
		Serial.println("reading comparator ...");
		comparatorRise = 0;
		ACSR |= 1 << ACIE;
		delay(1000);
		ACSR &= ~(1 << ACIE);
		Serial.print("comparatorRise = ");
		Serial.println(comparatorRise);
#endif

#ifdef USE_ADC_WITH_LOOP
		Serial.println("reading ADC ...");
		long start = millis();
		for(int i=0; i<INPUT_LEN; i++) {
			inputs[i] = analogRead(A0);
		}
		Serial.print("Took ");
		Serial.println(millis() - start);

		long total = 0;
		for(int i=0; i<INPUT_LEN; i++) {
			total += inputs[i];
		}
		int avg = total / INPUT_LEN;
		Serial.print("AVG = ");
		Serial.println(avg);

		for(int i=0; i<INPUT_LEN; i++) {
			printPad(avg - inputs[i]);
			if (i % 20 == 19) {
				Serial.println();
			}
		}
#endif

#ifdef USE_ADC_WITH_INTERRUPT
		bufferIndex = 0;
		long start = millis();
		digitalWrite(13, HIGH);

		// enable timer and interruption
		writeReg16(TCNT1, 0);
		TIMSK1 |= 1 << OCIE1A;
		// wait for bufferIndex == SAMPLES
		while (bufferIndex != SAMPLES);
		word cnt;
		readReg16(cnt, TCNT1);

		digitalWrite(13, LOW);
		Serial.print("Took ");
		Serial.println(millis() - start);
		Serial.print("TCNT1 ended at ");
		Serial.println(cnt);

		for (int i = 0; i < SAMPLES; i++) {
			vImag[i] = 0.0;
		}
		FFT.Windowing((double *)vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
//		Serial.println("Weighed data:");
//		PrintVector((double *)vReal, SAMPLES, SCL_TIME);
		FFT.Compute((double *)vReal, vImag, SAMPLES, FFT_FORWARD); /* Compute FFT */
//		Serial.println("Computed Real values:");
//		PrintVector((double *)vReal, SAMPLES, SCL_INDEX);
//		Serial.println("Computed Imaginary values:");
//		PrintVector(vImag, SAMPLES, SCL_INDEX);
		FFT.ComplexToMagnitude((double *)vReal, vImag, SAMPLES); /* Compute magnitudes */
//		Serial.println("Computed magnitudes:");
//		PrintVector((double *)vReal, (SAMPLES >> 1), SCL_FREQUENCY);
		double x = FFT.MajorPeak((double *)vReal, SAMPLES, samplingFrequency);
		Serial.print("Computed MajorPeak : ");
		Serial.println(x, 6);

		char *name;
		int error,octave;
		getNote(x, &name, &octave, &error);
		Serial.print(" => "); Serial.print(name);
		Serial.print(" ["); Serial.print(octave); Serial.print("] ~ ");
		Serial.println(error);
#endif
	}

}
#endif
