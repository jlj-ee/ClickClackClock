#include "serial_clock.h"
#include <Arduino.h>

// 7-segment display configurations for all the relevant digits
//    A
//  F   B
//    G
//  E   C
//    D   P
//  Order: ABCDEFGP (where P is the decimal point)
const uint8_t SerialClock::segments[10] = {
    0xFC,  // 0
    0x60,  // 1
    0xDA,  // 2
    0xF2,  // 3
    0x66,  // 4
    0xB6,  // 5
    0xBE,  // 6
    0xE0,  // 7
    0xFE,  // 8
    0xF6,  // 9
};

/*  SerialClock constructor
    Constructs a SerialClock object.
    Inputs:
      int dataPin, clockPin, strobePin, rightEnPin, leftEnPin : pin assignments
*/
SerialClock::SerialClock(int dataPin, int clockPin, int strobePin, int leftEnPin, int rightEnPin) {
  dataPin = dataPin;
  clockPin = clockPin;
  strobePin = strobePin;
  rightEnPin = rightEnPin;
  leftEnPin = leftEnPin;

  // segments[0] = 0xFC; // 0
  // segments[1] = 0x60; // 1
  // segments[2] = 0xDA; // 2
  // segments[3] = 0xF2; // 3
  // segments[4] = 0x66; // 4
  // segments[5] = 0xB6; // 5
  // segments[6] = 0xBE; // 6
  // segments[7] = 0xE0; // 7
  // segments[8] = 0xFE; // 8
  // segments[9] = 0xF6; // 9
}

/*  SerialClock latchData
    Pulses the strobe pin to latch data in to the clock.
*/
void SerialClock::latchData() {
  digitalWrite(strobePin, HIGH);  // Strobe is active high
  delayMicroseconds(SERIAL_PERIOD_US);
  digitalWrite(strobePin, LOW);
}

/*  SerialClock updateRight
    Pulses the output enable pin for minutes (or seconds) to update the display with the latched
   value.
*/
void SerialClock::updateRight() {
  digitalWrite(rightEnPin, LOW);  // Output enable is active low
  delay(OUTPUT_ENABLE_MS);
  digitalWrite(rightEnPin, HIGH);
}

/*  SerialClock updateLeft
    Pulses the output enable pin for hours (or minutes) to update the display with the latched
   value.
*/
void SerialClock::updateLeft() {
  digitalWrite(leftEnPin, LOW);  // Output enable is active low
  delay(OUTPUT_ENABLE_MS);
  digitalWrite(leftEnPin, HIGH);
}

/*  SerialClock writeBlank
    Writes the value blank the 7-segment display.
*/
void SerialClock::writeBlank() { shiftData((int)interleaveBytes(BLANK_DIGIT, ~BLANK_DIGIT)); }

/*  SerialClock writeDigit
    Writes a clock digit to the 7-segment display.
    Inputs:
      int digitVal  : the value of the digit (0-9)
      bool showP    : if true, shows the point for the digit
*/
void SerialClock::writeDigit(int digitVal, bool showP = false) {
  uint8_t data = segments[digitVal];
  if (showP) {
    data = data + B00000001;  // decimal point is lsb
  }
  shiftData((int)interleaveBytes(data, ~data));
}

/*  SerialClock updateDisplay
    Updates the clock display with new data according to the following format:
    [Left Left  : Right Right]
    [Tens Ones  : Tens  Ones]
    Inputs:
      int leftData  : data for the left-side digits (hours/minutes)
      int rightData : data for the right-side digits (minutes/seconds)
    Note: Sets the colon if left value is nonzero but hide leading zeros.
    e.g. valid display times: 10:01, _1:01, _ _ _15, _ _ _ _5
*/
void SerialClock::updateDisplay(int leftData, int rightData) {
  int data[2] = {leftData, rightData};
  int tens[2] = {leftData / 10, rightData / 10};
  int ones[2] = {leftData % 10, rightData % 10};
  bool showColon = false;
  // Write data to the daisy-chained drivers
  for (int i = 0; i < 2; i++) {
    if (showColon || tens[i] >= 1) {
      writeDigit(tens[i]);
      writeDigit(ones[i], showColon);
    } else {
      writeBlank();
      if (ones[i] > 0) {
        writeDigit(ones[i], showColon);
      } else {
        writeBlank();
      }
    }
    if (data[i] > 0) {
      showColon = true;
    }
  }
  // Latch data and enable outputs to display new values
  latchData();
  updateLeft();
  updateRight();
}

/*  SerialClock clearDisplay
    Blanks the clock display.
    Note: Flushes data from shift registers
*/
void SerialClock::clearDisplay(uint8_t mode = CLEAR_BOTH) {
  for (int i = 0; i < 4; i++) {
    writeBlank();
  }
  latchData();
  if (mode == CLEAR_BOTH || mode == CLEAR_LEFT) {
    updateLeft();
  }
  if (mode == CLEAR_BOTH || mode == CLEAR_RIGHT) {
    updateRight();
  }
}

/*  SerialClock shiftData
    Shifts data onto the serial bus.
    Inputs:
      int data          : value to be written to the serial bus
      uint8_t bitOrder  : {LSBFIRST, MSBFIRST} the order in which the bits in data will be written
      int bitCount      :  number of bits in data
*/
void SerialClock::shiftData(int data, uint8_t bitOrder = LSBFIRST, int bitCount = 16) {
  for (int i = 0; i < bitCount; i++) {
    if (bitOrder == LSBFIRST)
      digitalWrite(dataPin, !!(data & (1 << i)));
    else {
      digitalWrite(dataPin, !!(data & (1 << (bitCount - 1 - i))));
    }
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(SERIAL_PERIOD_US);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(SERIAL_PERIOD_US);
  }
}

/*  SerialClock interleaveBytes
    Interleaves the bits of two bytes to form one 16-bit word.
    ex) a = 0xF0, b = 0x0F => result = 0xAA55
    Inputs:
      uint8_t a : first byte
      uint8_t b : second byte
    Outputs:
      uint16_t result : the interleaved result of a and b
*/
uint16_t SerialClock::interleaveBytes(uint8_t a, uint8_t b) {
  uint16_t result = 0;
  for (int i = 7; i >= 0; i--) {
    result |= (a >> i) & 1;
    result <<= 1;
    result |= (b >> i) & 1;
    if (i != 0) {
      result <<= 1;
    }
  }
  return result;
}