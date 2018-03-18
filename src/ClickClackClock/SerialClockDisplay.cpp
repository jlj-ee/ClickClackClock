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

inline Segments& operator|=(Segments& lhs, const Segments& rhs) {
  return lhs = static_cast<Segments>(static_cast<uint8_t>(lhs) |
                                     static_cast<uint8_t>(rhs));
}

void SerialClockDisplay::begin(const SerialDisplayConfig* display_config) {
  // Set private members
  _config = display_config;

  // Configure pins
  pinMode(_config->data_pin, OUTPUT);
  pinMode(_config->clock_pin, OUTPUT);
  pinMode(_config->strobe_pin, OUTPUT);
  pinMode(_config->leften_pin, OUTPUT);
  pinMode(_config->righten_pin, OUTPUT);

  // Get register/bitmask information for serial pins
  _clock_reg = portOutputRegister(digitalPinToPort(_config->clock_pin));
  _clock_bit = digitalPinToBitMask(_config->clock_pin);
  _data_reg = portOutputRegister(digitalPinToPort(_config->data_pin));
  _data_bit = digitalPinToBitMask(_config->data_pin);
}

void SerialClockDisplay::writeBuffer(Segments display_val, uint8_t loc,
                                     bool show_p) {
  // Point is LSB of segments; set it if necessary
  if (show_p) {
    display_val |= kSDot;
  }
  _display[loc] = display_val;
}

void SerialClockDisplay::writeBufferNumeric(uint8_t digit_val, uint8_t loc,
                                            bool show_p) {
  // Select correct segment configuration for the given digit value
  Segments numbers[10] = {kS0, kS1, kS2, kS3, kS4, kS5, kS6, kS7, kS8, kS9};
  if (digit_val < 10) {
    writeBuffer(numbers[digit_val], loc, show_p);
  } else {
    // If no matching configuration was found, clear the display.
    writeBuffer(kSBlank, loc);
  }
}

void SerialClockDisplay::writeBufferTime(uint8_t left_data,
                                         uint8_t right_data) {
  uint8_t data[NUM_SECTIONS] = {left_data, right_data};
  // Extract tens digits and ones digits separately
  uint8_t tens[NUM_SECTIONS] = {static_cast<uint8_t>(left_data / 10),
                                static_cast<uint8_t>(right_data / 10)};
  uint8_t ones[NUM_SECTIONS] = {static_cast<uint8_t>(left_data % 10),
                                static_cast<uint8_t>(right_data % 10)};
  bool show_colon = false;
  // Write data to the buffer to fill all four digits
  uint8_t j = 0;
  for (int i = 0; i < NUM_SECTIONS; i++) {
    if (show_colon || tens[i] >= 1) {
      writeBufferNumeric(tens[i], j);
      j++;
      writeBufferNumeric(ones[i], j, show_colon);
    } else {
      writeBuffer(kSBlank, j);
      j++;
      if (ones[i] > 0) {
        writeBufferNumeric(ones[i], j, show_colon);
      } else {
        writeBuffer(kSBlank, j);
      }
    }
    if (data[i] > 0) {
      show_colon = true;
    }
    j++;
  }
}

void SerialClockDisplay::displayBuffer(bool left, bool right) {
  if (left || right) {
    // Shift data onto the serial bus
    for (int i = 0; i < NUM_DISPLAYS; i++) {
      shiftData(static_cast<int>(interleaveBytes(_display[i], ~_display[i])));
    }
    // Latch data in and enable outputs to display new data
    latchData();
  }
  if (left) {
    updateLeft();
  }
  if (right) {
    updateRight();
  }
}

void SerialClockDisplay::clearDisplay(bool left, bool right) {
  // Insert blanks according to the given mode
  if (left) {
    _display[0] = kSBlank;
    _display[1] = kSBlank;
  }
  if (right) {
    _display[2] = kSBlank;
    _display[3] = kSBlank;
  }
  // Push new (blank) data to the displays
  displayBuffer(left, right);
}

void SerialClockDisplay::latchData(void) {
  digitalWrite(_config->strobe_pin, HIGH);
  delayMicroseconds(_config->clock_period_us);
  digitalWrite(_config->strobe_pin, LOW);
}

void SerialClockDisplay::updateRight(void) {
  digitalWrite(_config->righten_pin, LOW);
  delay(_config->en_pulse_ms);
  digitalWrite(_config->righten_pin, HIGH);
}

void SerialClockDisplay::updateLeft(void) {
  digitalWrite(_config->leften_pin, LOW);
  delay(_config->en_pulse_ms);
  digitalWrite(_config->leften_pin, HIGH);
}

Segments* SerialClockDisplay::readDisplay(void) {
  static Segments displayCopy[NUM_DISPLAYS];
  for (int i = 0; i < NUM_DISPLAYS; i++) {
    displayCopy[i] = _display[i];
  }
  return displayCopy;
}

/*===========================================================================*/
// Low-level functions

// Bit-bang implementation of a serial write since speed is not an issue
void SerialClockDisplay::shiftData(int data, bool lsb_first, int bit_count) {
  // Construct data buffer according to bit order
  bool data_buffer[bit_count];
  for (int i = 0; i < bit_count; i++) {
    if (lsb_first)
      data_buffer[i] = !!(data & (1 << i));
    else {
      data_buffer[i] = !!(data & (1 << (bit_count - 1 - i)));
    }
  }

  // // Write data and clock
  for (int i = 0; i < bit_count; i++) {
    if (data_buffer[i]) {
      *_data_reg |= _data_bit;  // HIGH
    } else {
      *_data_reg &= ~_data_bit;  // LOW
    }
    // Cycle clock
    *_clock_reg |= _clock_bit;  // HIGH
    delayMicroseconds(_config->clock_period_us / 2);
    *_clock_reg &= ~_clock_bit;  // LOW
    delayMicroseconds(_config->clock_period_us / 2);
  }
  *_data_reg &= ~_data_bit;
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