/*

*/
#ifndef _SERIAL_TIME_H_
#define _SERIAL_TIME_H_

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
  int dataPin, clockPin, strobePin, rightEnPin, leftEnPin;
  uint16_t interleaveBytes(uint8_t a, uint8_t b);
  void shiftData(int val, uint8_t bitOrder, int bitCount);

 public:
  SerialClock(int dataPin, int clockPin, int strobePin, int leftEnPin, int rightEnPin);
  void updateDisplay(int leftData, int rightData);
  void clearDisplay(uint8_t mode);
  void writeDigit(int digitVal, bool showP);
  void writeBlank();
  void latchData();
  void updateRight();
  void updateLeft();
};

#endif