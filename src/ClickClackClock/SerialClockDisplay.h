/*!
 * @file SerialClockDisplay.h
 * @brief      Class and function prototypes for a clock display driver.
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

#ifndef _SERIAL_CLOCK_DISPLAY_H_
#define _SERIAL_CLOCK_DISPLAY_H_

#include <stdint.h>

#define NUM_DISPLAYS 4
#define NUM_SECTIONS 2

/*!
 * @brief       7-segment display configurations.
 *                            A
 *                          F   B
 *                            G
 *                          E   C
 *                            D   P
 *              Order: ABCDEFGP (where P is the decimal point)
 */
typedef enum eSegments {
  kS0 = 0xFC,     //!< B11111100 => 0
  kS1 = 0x60,     //!< B01100000 => 1
  kS2 = 0xDA,     //!< B11001010 => 2
  kS3 = 0xF2,     //!< B11110010 => 3
  kS4 = 0x66,     //!< B01100110 => 4
  kS5 = 0xB6,     //!< B10110110 => 5
  kS6 = 0xBE,     //!< B10111110 => 6
  kS7 = 0xE0,     //!< B11100000 => 7
  kS8 = 0xFE,     //!< B11111110 => 8
  kS9 = 0xF6,     //!< B11110110 => 9
  kSDot = 0x01,   //!< B00000001 => Point
  kSBlank = 0x00  //!< B00000000 => All off
} Segments;

/*! @brief       Bitwise or assignment operator for Segments */
inline Segments& operator|=(Segments& lhs, const Segments& rhs);

/*! @brief      Blanking modes for the SerialClockDisplay. */
typedef enum eClearMode {
  kClearBoth = 0,  //!< Clear both halves.
  kClearLeft = 1,  //!< Clear left half.
  kClearRight = 2  //!< Clear right half.
} ClearMode;

/*! @brief      Digital active level. */
typedef enum eActiveLevel {
  kActiveLow = 0,  //!< Assertion = digital low
  kActiveHigh = 1  //!< Assertion = digital high
} ActiveLevel;

/*!
 * @brief      Structure to encapsulate serial display configuration parameters.
 */
struct SerialDisplayConfig {
  uint8_t data_pin;        //!< Serial data pin.
  uint8_t clock_pin;       //!< Serial clock pin.
  uint8_t strobe_pin;      //!< Strobe (latch) pin.
  uint8_t leften_pin;      //!< Output enable pin for left half.
  uint8_t righten_pin;     //!< Output enable pin for right half.
  int clock_period_us;     //!< Serial clock period in microseconds.
  int en_pulse_ms;         //!< Output enable pulse width in milliseconds.
  ActiveLevel strobe_pol;  //!< Strobe (latch) polarity.
  ActiveLevel en_pol;      //!< Output enable polarity.
};

/*===========================================================================*/
// Class definition
class SerialClockDisplay {
  // Public members and functions
 public:
  /*!
   * @brief      Configures and starts the SerialClockDisplay.
   *
   * @param      display_config  Pointer to the structure used to configure the
   *                             SerialClockDisplay.
   */
  void begin(const SerialDisplayConfig* display_config);

  /*!
   * @brief      Writes an arbitrary segment configuration to the buffer.
   *
   * @param      display_val  8-bit segment configuration.
   * @param      loc          Index to write
   * @param      show_p       If true, shows the decimal point/colon.
   */
  void writeBuffer(Segments display_val, uint8_t loc, bool show_p = false);

  /*!
   * @brief      Writes a clock digit value to the buffer.
   *
   * @param      digit_val  8-bit clock digit value (0-9).
   * @param      loc          Index to write
   * @param      show_p     If true, shows the decimal point/colon.
   */
  void writeBufferNumeric(uint8_t digit_val, uint8_t loc, bool show_p = false);

  /*!
   * @brief       Writes the buffer to the serial bus, latches it into the
   *              drivers, and enables outputs to update the displayed values.
   */
  void displayBuffer(void);

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
  void displayTime(uint8_t left_data, uint8_t right_data);

  /*!
   * @brief      Clears the clock display.
   *
   * @param      mode  Chooses whether left/right/both displays are cleared.
   *                   (CLEAR_BOTH or CLEAR_LEFT or CLEAR_RIGHT)
   */
  void clearDisplay(ClearMode mode = kClearBoth);

  /*!
   * @brief      Pulses the strobe pin to latch in data on the serial bus.
   */
  void latchData(void);

  /*!
   * @brief      Pulses the right enable pin to display data on that side.
   */
  void updateRight(void);

  /*!
   * @brief      Pulses the left enable pin to display data on that side.
   */
  void updateLeft(void);

  /**
   * @brief      Returns a pointer to an array holding the display values.
   */
  Segments* readDisplay(void);

  /*=========================================================================*/
  // Private members and low-level functions
 private:
  const SerialDisplayConfig* _config;  //!< Pointer to the configuration.
  Segments _display[NUM_DISPLAYS];     //!< Buffer of values on the display.
  volatile uint8_t* _clock_reg;        //!< Pointer to the clock pin register
  volatile uint8_t* _data_reg;         //!< Pointer to the data pin register
  uint8_t _clock_bit, _data_bit;       //!< Clock and data pin bit masks

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

  /*!
   * @brief      Interleaves the bits of bytes a and b to form one word.
   *             Starts with the first bit of a, then the first bit of b, etc.
   *             For example: a = 0xF0, b = 0x0F => result = 0xAA55
   *
   * @param      a     First byte.
   * @param      b     Second byte.
   *
   * @return     The interleaved result of a and b.
   */
  uint16_t interleaveBytes(uint8_t a, uint8_t b);
};

#endif  //_SERIAL_CLOCK_DISPLAY_H_