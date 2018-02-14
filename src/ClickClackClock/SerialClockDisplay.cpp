/*!
 * @file       SerialClockDisplay.cpp
 * @brief      Clock display driver.
 * 
 *             Clock display consists of four 7-segment displays that are 
 *             daisy-chained and driven by serial-input latched source drivers 
 *             like the MIC5891.
 *
 * @author     Jaime Jimenez
 * @author     CJ Valle
 *
 * @version    0.1
 * @date       February 2018
 */

#include "SerialClockDisplay.h"
#include <Arduino.h>

/*===========================================================================*/
// Public functions

void SerialClockDisplay::begin(SerialDisplayConfig *display_config) {
  // Set private members
  config_ = display_config;

  // Configure pins
  pinMode(config_->data_pin, OUTPUT);
  pinMode(config_->clock_pin, OUTPUT);
  pinMode(config_->strobe_pin, OUTPUT);
  pinMode(config_->leften_pin, OUTPUT);
  pinMode(config_->righten_pin, OUTPUT);
}

void SerialClockDisplay::writeArbitrary(Segments display_val, bool show_p) {
  // Cast enum to raw byte
  uint8_t raw_bits = display_val;
  // Point is LSB of segments; set it if necessary
  if (show_p) {
    raw_bits = raw_bits | S_DOT;
  }
  // Shift data onto the serial bus
  shiftData((int)interleaveBytes(raw_bits, ~raw_bits));
}

void SerialClockDisplay::writeNumeric(uint8_t digit_val, bool show_p) {
  // Select correct segment configuration for the given digit value
  Segments numbers[10] = {S_0, S_1, S_2, S_3, S_4, S_5, S_6, S_7, S_8, S_9};
  if (digit_val < 10) {
    writeArbitrary(numbers[digit_val], show_p);
  } else {
    // If no matching configuration was found, clear the display.
    writeArbitrary(S_BLANK);
  }
}

void SerialClockDisplay::updateTime(uint8_t left_data, uint8_t right_data) {
  int data[2] = {left_data, right_data};
  // Extract tens digits and ones digits separately
  int tens[2] = {left_data / 10, right_data / 10};
  int ones[2] = {left_data % 10, right_data % 10};
  bool show_colon = false;
  // Write data to the daisy-chained drivers to fill all four digits
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

void SerialClockDisplay::clearDisplay(ClearMode mode) {
  // Write blanks to all digits
  for (int i = 0; i < 4; i++) {
    writeArbitrary(S_BLANK);
  }
  latchData();
  // Only enable (display new data) according to the given mode
  if (mode == CLEAR_BOTH || mode == CLEAR_LEFT) {
    updateLeft();
  }
  if (mode == CLEAR_BOTH || mode == CLEAR_RIGHT) {
    updateRight();
  }
}

void SerialClockDisplay::latchData(void) {
  digitalWrite(config_->strobe_pin, HIGH);
  delayMicroseconds(config_->clock_period_us);
  digitalWrite(config_->strobe_pin, LOW);
}

void SerialClockDisplay::updateRight(void) {
  digitalWrite(config_->righten_pin, LOW);
  delay(config_->en_pulse_ms);
  digitalWrite(config_->righten_pin, HIGH);
}

void SerialClockDisplay::updateLeft(void) {
  digitalWrite(config_->leften_pin, LOW);
  delay(config_->en_pulse_ms);
  digitalWrite(config_->leften_pin, HIGH);
}

/*===========================================================================*/
// Low-level functions

// Simple bit-bang implementation of a serial write since speed is not an issue
void SerialClockDisplay::shiftData(int data, bool lsb_first, int bit_count) {
  // Mask off bits according to bit order and set data pin
  for (int i = 0; i < bit_count; i++) {
    if (lsb_first)
      digitalWrite(config_->data_pin, !!(data & (1 << i)));
    else {
      digitalWrite(config_->data_pin, !!(data & (1 << (bit_count - 1 - i))));
    }
    // Cycle clock
    digitalWrite(config_->clock_pin, HIGH);
    delayMicroseconds(config_->clock_period_us);
    digitalWrite(config_->clock_pin, LOW);
    delayMicroseconds(config_->clock_period_us);
  }
}

uint16_t SerialClockDisplay::interleaveBytes(uint8_t a, uint8_t b) {
  uint16_t result = 0;
  for (int i = 7; i >= 0; i--) {
    // Mask off the next bit of 'a' and copy to result
    result |= (a >> i) & 1;
    // Shift the result to make room
    result <<= 1;
    // Mask off the next bit of 'b' and copy to result
    result |= (b >> i) & 1;
    // Shift the result to make room unless we have reached the end
    if (i != 0) {
      result <<= 1;
    }
  }
  return result;
}