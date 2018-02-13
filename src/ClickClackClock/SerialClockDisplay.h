/*!
 * @file SerialClock.h
 * @brief      Class and functions to control a clock display that consists of
 *             four 7-segment displays that are daisy-chained serially.
 *
 * @author     Jaime Jimenez
 * @author     CJ Valle
 *
 * @version    0.1
 * @date       February 2018
 */
#ifndef _SERIAL_CLOCK_H_
#define _SERIAL_CLOCK_H_

#include <stdint.h>

/*===========================================================================*/
// Class definition
class SerialClock {
  
 // Public members and functions
 public:
  /*!
   * @brief       7-segment display configurations for all the relevant digits
   *                                    A
   *                                  F   B
   *                                    G
   *                                  E   C
   *                                    D   P
   *              Order: ABCDEFGP (where P is the decimal point)
   */
  enum Segments {
    S_0 = 0xFC,     ///< B11111100 => 0
    S_1 = 0x60,     ///< B01100000 => 1
    S_2 = 0xDA,     ///< B11001010 => 2
    S_3 = 0xF2,     ///< B11110010 => 3
    S_4 = 0x66,     ///< B01100110 => 4
    S_5 = 0xB6,     ///< B10110110 => 5
    S_6 = 0xBE,     ///< B10111110 => 6
    S_7 = 0xE0,     ///< B11100000 => 7
    S_8 = 0xFE,     ///< B11111110 => 8
    S_9 = 0xF6,     ///< B11110110 => 9
    S_DOT = 0x01,   ///< B00000001 => Point
    S_BLANK = 0x00  ///< B00000000 => All off
  };

  /*! @brief      Blanking modes for the SerialClock. */
  enum ClearMode {
    CLEAR_BOTH = 0,  ///< Clear both halves
    CLEAR_LEFT = 1,  ///< Clear left half
    CLEAR_RIGHT = 2  ///< Clear right half
  };

  /*!
   * @brief      Constructs a SerialClock object.
   *
   * @param      data_pin      Serial data pin.
   * @param      clock_pin     Serial clock pin.
   * @param      strobe_pin    Active-high strobe (latch) pin.
   * @param      left_en_pin   Active-low output enable pin for left half.
   * @param      right_en_pin  ACtive-low output enable pin for right half.
   */
  SerialClock(int data_pin, int clock_pin, int strobe_pin, int left_en_pin,
              int right_en_pin, int clock_period_us = 4,
              int strobe_pulse_ms = 200);

  /*!
   * @brief      Updates the clock display with new time data, according to the
   *             following format:
   *           + [Left Left  : Right Right]
   *           + [Tens Ones  : Tens  Ones ]
   *           + Note: Sets the colon if left value is nonzero but hides leading
   *             zeros. e.g. valid display times:
   *           + 10:01, _1:01, _ _ _15, _ _ _ _5
   *
   * @param      left_data   Data for the left digits (hours or minutes).
   * @param      right_data  Data for the right digits (minutes or seconds).
   */
  void updateTime(uint8_t left_data, uint8_t right_data);

  /*!
   * @brief      Clears the clock display and flushes data from serial bus.
   *
   * @param      mode  Chooses whether left/right/both displays are cleared.
   *                   (CLEAR_BOTH or CLEAR_LEFT or CLEAR_RIGHT)
   */
  void clearDisplay(ClearMode mode = CLEAR_BOTH);

  /*!
   * @brief      Writes a clock digit value to the serial bus
   *
   * @param      digit_val  8-bit clock digit value (0-9)
   * @param      show_p     If true, shows the decimal point/colon
   */
  void writeNumeric(uint8_t digit_val, bool show_p = false);

  /*!
   * @brief      Writes an arbitrary segment configuration to the serial bus
   *
   * @param      display_val  8-bit segment configuration
   * @param      show_p       If true, shows the decimal point/colon
   */
  void writeArbitrary(Segments display_val, bool show_p = false);

  /*!
   * @brief      Pulses the strobe pin to latch in data on the serial bus
   */
  void latchData(void);

  /*!
   * @brief      Pulses the right output enable pin to display data on that side
   */
  void updateRight(void);

  /*!
   * @brief      Pulses the left output enable pin to display data on that side
   */
  void updateLeft(void);

/*===========================================================================*/
// Private members and low-level functions
 private:
  int data_pin_, clock_pin_, strobe_pin_, right_en_pin_, left_en_pin_; //!< Pins
  int clock_period_us_; //!< Serial clock period in microseconds 
  int strobe_pulse_ms_; //!< Latch pulse width

  /*!
   * @brief      Interleaves the bits of bytes a and b to form one word.
   *             Starts with the first bit of a, then the first bit of b, etc.
   *             For example: a = 0xF0, b = 0x0F => result = 0xAA55
   *
   * @param      a     First byte
   * @param      b     Second byte
   *
   * @return     The interleaved result of a and b.
   */
  uint16_t interleaveBytes(uint8_t a, uint8_t b);

  /*!
   * @brief      Shifts data onto the serial bus.
   *
   * @param      val        Value to be written to the serial bus.
   * @param      lsb_first  If true, the data is shifted in starting with the
   *                        least significant bit. Otherwise, it is shifted in
   *                        starting with the most significant bit.
   * @param      bit_count  The number of bits of data to be shifted.
   */
  void shiftData(int val, bool lsb_first = true, int bit_count = 16);
};

#endif  //_SERIAL_CLOCK_H_