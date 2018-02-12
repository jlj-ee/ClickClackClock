/*

*/
#ifndef _SERIAL_CLOCK_H_
#define _SERIAL_CLOCK_H_

#include <stdint.h>
#include <Arduino.h>

// Communication constants
#define SERIAL_PERIOD_US 4    // Arbitrary 250kHz data rate
#define OUTPUT_ENABLE_MS 200  // Arbitrary output enable duration

// Display modes
#define CLEAR_LEFT 0
#define CLEAR_RIGHT 1
#define CLEAR_BOTH 2
#define BLANK_DIGIT 0x00

class SerialClock {
  const static uint8_t segments[10];
  int data_pin_, clock_pin_, strobe_pin_, right_en_pin_, left_en_pin_;
  uint16_t interleaveBytes(uint8_t a, uint8_t b);
  void shiftData(int val, uint8_t bitOrder = LSBFIRST, int bitCount = 16);

 public:
  SerialClock(int data_pin, int clock_pin, int strobe_pin, int left_en_pin, int right_en_pin);
  void updateDisplay(int left_data, int right_data);
  void clearDisplay(uint8_t mode = CLEAR_BOTH);
  void writeDigit(int digit_val, bool show_p = false);
  void writeBlank();
  void latchData();
  void updateRight();
  void updateLeft();
};

#endif