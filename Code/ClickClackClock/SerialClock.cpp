#include "SerialClock.h"
#include <Arduino.h>

// 7-segment display configurations for all the relevant digits
//    A
//  F   B
//    G
//  E   C
//    D   P
//  Order: ABCDEFGP (where P is the decimal point)
const uint8_t SerialClock::segments[10] = {
    0xFC,  // B11111100 => 0
    0x60,  // B01100000 => 1
    0xDA,  // B11001010 => 2
    0xF2,  // B11110010 => 3
    0x66,  // B01100110 => 4
    0xB6,  // B10110110 => 5
    0xBE,  // B10111110 => 6
    0xE0,  // B11100000 => 7
    0xFE,  // B11111110 => 8
    0xF6,  // B11110110 => 9
};

/*  SerialClock constructor
    Constructs a SerialClock object.
    Inputs:
      int data_pin, clock_pin, strobe_pin, right_en_pin, left_en_pin : pin assignments
*/
SerialClock::SerialClock(int data_pin, int clock_pin, int strobe_pin, int left_en_pin, int right_en_pin) {
  data_pin_ = data_pin;
  clock_pin_ = clock_pin;
  strobe_pin_ = strobe_pin;
  right_en_pin_ = right_en_pin;
  left_en_pin_ = left_en_pin;
}

/*  SerialClock latchData
    Pulses the strobe pin to latch data in to the clock.
*/
void SerialClock::latchData() {
  digitalWrite(strobe_pin_, HIGH);  // Strobe is active high
  delayMicroseconds(SERIAL_PERIOD_US);
  digitalWrite(strobe_pin_, LOW);
}

/*  SerialClock updateRight
    Pulses the output enable pin for minutes (or seconds) to update the display with the latched
   value.
*/
void SerialClock::updateRight() {
  digitalWrite(right_en_pin_, LOW);  // Output enable is active low
  delay(OUTPUT_ENABLE_MS);
  digitalWrite(right_en_pin_, HIGH);
}

/*  SerialClock updateLeft
    Pulses the output enable pin for hours (or minutes) to update the display with the latched
   value.
*/
void SerialClock::updateLeft() {
  digitalWrite(left_en_pin_, LOW);  // Output enable is active low
  delay(OUTPUT_ENABLE_MS);
  digitalWrite(left_en_pin_, HIGH);
}

/*  SerialClock writeBlank
    Writes the value blank the 7-segment display.
*/
void SerialClock::writeBlank() { shiftData((int)interleaveBytes(BLANK_DIGIT, ~BLANK_DIGIT)); }

/*  SerialClock writeDigit
    Writes a clock digit to the 7-segment display.
    Inputs:
      int digit_val  : the value of the digit (0-9)
      bool show_p    : if true, shows the point for the digit
*/
void SerialClock::writeDigit(int digit_val, bool show_p) {
  uint8_t data = segments[digit_val];
  if (show_p) {
    data = data + B00000001;  // decimal point is lsb
  }
  shiftData((int)interleaveBytes(data, ~data));
}

/*  SerialClock updateDisplay
    Updates the clock display with new data according to the following format:
    [Left Left  : Right Right]
    [Tens Ones  : Tens  Ones]
    Inputs:
      int left_data  : data for the left-side digits (hours/minutes)
      int right_data : data for the right-side digits (minutes/seconds)
    Note: Sets the colon if left value is nonzero but hide leading zeros.
    e.g. valid display times: 10:01, _1:01, _ _ _15, _ _ _ _5
*/
void SerialClock::updateDisplay(int left_data, int right_data) {
  int data[2] = {left_data, right_data};
  int tens[2] = {left_data / 10, right_data / 10};
  int ones[2] = {left_data % 10, right_data % 10};
  bool show_colon = false;
  // Write data to the daisy-chained drivers
  for (int i = 0; i < 2; i++) {
    if (show_colon || tens[i] >= 1) {
      writeDigit(tens[i]);
      writeDigit(ones[i], show_colon);
    } else {
      writeBlank();
      if (ones[i] > 0) {
        writeDigit(ones[i], show_colon);
      } else {
        writeBlank();
      }
    }
    if (data[i] > 0) {
      show_colon = true;
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
void SerialClock::clearDisplay(uint8_t mode) {
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
      uint8_t bit_order  : {LSBFIRST, MSBFIRST} the order in which the bits in data will be written
      int bit_count      :  number of bits in data
*/
void SerialClock::shiftData(int data, uint8_t bit_order, int bit_count) {
  for (int i = 0; i < bit_count; i++) {
    if (bit_order == LSBFIRST)
      digitalWrite(data_pin_, !!(data & (1 << i)));
    else {
      digitalWrite(data_pin_, !!(data & (1 << (bit_count - 1 - i))));
    }
    digitalWrite(clock_pin_, HIGH);
    delayMicroseconds(SERIAL_PERIOD_US);
    digitalWrite(clock_pin_, LOW);
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