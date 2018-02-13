#include "SerialClock.h"
#include <Arduino.h>

/*===========================================================================*/
// Class constructor

SerialClock::SerialClock(int data_pin, int clock_pin, int strobe_pin,
                         int left_en_pin, int right_en_pin, int clock_period_us,
                         int strobe_pulse_ms) {
  // Set private members
  data_pin_ = data_pin;
  clock_pin_ = clock_pin;
  strobe_pin_ = strobe_pin;
  right_en_pin_ = right_en_pin;
  left_en_pin_ = left_en_pin;
  clock_period_us_ = clock_period_us;
  strobe_pulse_ms_ = strobe_pulse_ms;

  // Configure pins
}

/*===========================================================================*/
// Public functions

void SerialClock::writeArbitrary(Segments display_val, bool show_p) {
  // Cast enum to raw byte
  uint8_t raw_bits = display_val;
  // Point is LSB of segments; set it if necessary
  if (show_p) {
    raw_bits = raw_bits | S_DOT;
  }
  // Shift data onto the serial bus
  shiftData((int)interleaveBytes(raw_bits, ~raw_bits));
}

void SerialClock::writeNumeric(uint8_t digit_val, bool show_p) {
  // Select correct segment configuration for the given digit value
  Segments numbers[10] = {S_0, S_1, S_2, S_3, S_4, S_5, S_6, S_7, S_8, S_9};
  if (digit_val < 10) {
    writeArbitrary(numbers[digit_val], show_p);  
  }
  else {
    // If no matching configuration was found, clear the display.
    writeArbitrary(S_BLANK);
  }
}

void SerialClock::updateTime(uint8_t left_data, uint8_t right_data) {
  int data[2] = {left_data, right_data};
  // Extract tens digits and ones digits separately
  int tens[2] = {left_data / 10, right_data / 10};
  int ones[2] = {left_data % 10, right_data % 10};
  bool show_colon = false;
  // Write data to the daisy-chained drivers
  for (int i = 0; i < 2; i++) {
    if (show_colon || tens[i] >= 1) {
      writeNumeric(tens[i]);
      writeNumeric(ones[i], show_colon);
    } else {
      writeArbitrary(S_BLANK);
      if (ones[i] > 0) {
        writeNumeric(ones[i], show_colon);
      } else {
        writeArbitrary(S_BLANK);
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


void SerialClock::clearDisplay(ClearMode mode) {
  // Write blanks to all digits
  for (int i = 0; i < 4; i++) {
    writeArbitrary(S_BLANK);
  }
  latchData();
  // Only enable (apply) according to the given mode
  if (mode == CLEAR_BOTH || mode == CLEAR_LEFT) {
    updateLeft();
  }
  if (mode == CLEAR_BOTH || mode == CLEAR_RIGHT) {
    updateRight();
  }
}

void SerialClock::latchData(void) {
  digitalWrite(strobe_pin_, HIGH);  // Strobe is active high
  delayMicroseconds(clock_period_us_);
  digitalWrite(strobe_pin_, LOW);
}

void SerialClock::updateRight(void) {
  digitalWrite(right_en_pin_, LOW);  // Output enable is active low
  delay(strobe_pulse_ms_);
  digitalWrite(right_en_pin_, HIGH);
}

void SerialClock::updateLeft(void) {
  digitalWrite(left_en_pin_, LOW);  // Output enable is active low
  delay(strobe_pulse_ms_);
  digitalWrite(left_en_pin_, HIGH);
}

/*===========================================================================*/
// Low-level functions

// Simple bit-bang implementation since speed is not an issue 
void SerialClock::shiftData(int data, bool lsb_first, int bit_count) {
  // Mask off bits according to bit order and set data pin
  for (int i = 0; i < bit_count; i++) {
    if (lsb_first)
      digitalWrite(data_pin_, !!(data & (1 << i)));
    else {
      digitalWrite(data_pin_, !!(data & (1 << (bit_count - 1 - i))));
    }
    // Cycle clock
    digitalWrite(clock_pin_, HIGH);
    delayMicroseconds(clock_period_us_);
    digitalWrite(clock_pin_, LOW);
    delayMicroseconds(clock_period_us_);
  }
}

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