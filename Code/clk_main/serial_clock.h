/*

*/
#ifndef _SERIAL_TIME_H_
#define _SERIAL_TIME_H_

#include <Arduino.h>

#define SERIAL_PERIOD_US 4 // Arbitrary 250kHz data rate
#define OUTPUT_ENABLE_MS 200 // Arbitrary
#define BLANK_DIGIT B00000000

class SerialClock{
    const static uint8_t segments[10];
    int dataPin, clockPin, strobePin, rightEnPin, leftEnPin;
    uint16_t interleaveBytes(uint8_t a, uint8_t b);
    void shiftData(int val, uint8_t bitOrder, int bitCount);
    void writeDigit(int digitVal, bool showP);
    public:
        SerialClock(int dataPin, int clockPin, int strobePin, int leftEnPin, int rightEnPin);
        void writeBlank();
        void writeDisplay(int leftData, int rightData);
        void latchData();
        void updateRight();
        void updateLeft();
};



#endif