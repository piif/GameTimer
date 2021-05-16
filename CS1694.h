#include "Arduino.h"

extern byte segmentRam[14];

extern byte keys[5];

#define DISPLAY_MODE_COMMAND 0b00000000
#define DISPLAY_CONTROL_COMMAND 0b10000000
#define OPERATION_MODE_COMMAND 0b01000000
#define SET_ADDRESS_COMMAND 0b11000000

#define MODE_6_GRIDS_11_SEGS 0b10
#define MODE_7_GRIDS_10_SEGS 0b11

#define OPERATION_READ  0b10
#define OPERATION_WRITE 0b00

#define ADDRESS_FIXED       0b100
#define ADDRESS_AUTO_PLUS_1 0b000

#define NORMAL_MODE 0b0000
#define TEST_MODE   0b1000

#define DISPLAY_OFF 0b0000
#define DISPLAY_ON  0b1000

void setup_CS1694();

void sendCommand(byte command);

void sendWriteCommand(byte len, byte data[]);

void sendReadCommand(byte len, byte data[]);

#define displayMode(mode) sendCommand(DISPLAY_MODE_COMMAND | (mode))

#define displayControl(display, brightness) sendCommand(DISPLAY_CONTROL_COMMAND | (display) | (brightness))

#define setAddress(address) sendCommand(SET_ADDRESS_COMMAND | (address))

void readKeys();

void sendSegments();
