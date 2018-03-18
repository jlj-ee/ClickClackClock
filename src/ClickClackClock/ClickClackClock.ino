#include <Arduino.h>
#include "EnableInterrupt.h"
#include "RTClib.h"
#include "SerialClockDisplay.h"

/*===========================================================================*/
// Macros for debug
#define DEBUG 1  // Set to 0 to disable debug functions

#define debug_start(baud)          \
  do {                             \
    if (DEBUG) Serial.begin(baud); \
  } while (0)
#define debug_println(...)                  \
  do {                                      \
    if (DEBUG) Serial.println(__VA_ARGS__); \
  } while (0)
#define debug_print(...)                  \
  do {                                    \
    if (DEBUG) Serial.print(__VA_ARGS__); \
  } while (0)
#define debug_printDisplay()   \
  do {                         \
    if (DEBUG) printDisplay(); \
  } while (0)

/*===========================================================================*/
// Some enum definitions for convenience

// Output pin designations
typedef enum eOutputPins {
  kLeftEnPin = 9,    //!< Active-low output enable pin (!OE) for left side
  kRightEnPin = 10,  //!< Active-low output enable pin (!OE) for right side
  kStrobePin = 11,   //!< Active-high strobe pin (latch) for serial data
  kClockPin = 12,    //!< Active-high clock pin for serial data
  kDataPin = 13      //!< Data pin for serial data
} OutputPins;

// Input pin designations
typedef enum eInputPin {
  kClockModePin = 2,
  kCountModePin = 3,
  kCtrl1Pin = 4,
  kCtrl2Pin = 5,
  kUpPin = 6,
  kDownPin = 7
} InputPins;

// Operating modes
typedef enum eModes {
  kModeClock,
  kModeClockSetHour,
  kModeClockSetMinute,
  kModeStopwatchSet,
  kModeStopwatchRun,
  kModeStopwatchPause,
  kModeTimerSet,
  kModeTimerRun,
  kModeTimerPause
} Modes;

/*============================================================================*/
// Local constants and variables
const uint8_t kInputPins[] = {kClockModePin, kCountModePin, kCtrl1Pin,
                              kCtrl1Pin,     kUpPin,        kDownPin};
int nInputPins = sizeof(kInputPins) / sizeof(kInputPins[0]);
DateTime last_time, current_time;
bool update_left, update_right;
RTC_DS1307 rtc;
const SerialDisplayConfig kConfig = {.data_pin = kDataPin,
                                     .clock_pin = kClockPin,
                                     .strobe_pin = kStrobePin,
                                     .leften_pin = kLeftEnPin,
                                     .righten_pin = kRightEnPin,
                                     .clock_period_us = 4,
                                     .en_pulse_ms = 200,
                                     .strobe_pol = kActiveHigh,
                                     .en_pol = kActiveLow};
SerialClockDisplay display;
volatile Modes current_mode;
volatile int light_threshold;

/*============================================================================*/
void setup() {
  debug_start(9600);

  display.begin(&kConfig);

  // Set up input pins
  for (int i = 0; i < nInputPins; i++) {
    pinMode(kInputPins[i], INPUT_PULLUP);
    enableInterrupt(kInputPins[i] | PINCHANGEINTERRUPT, buttonHandler, FALLING);
  }

  rtc.begin();  // Always returns true
  if (!rtc.isrunning()) {
    debug_println("RTC is NOT running! :(");
  } else {
    debug_println("RTC is running! :)");
  }
  // Set the RTC to the date & time this sketch was compiled:
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  current_mode = kModeClock;
}

void buttonHandler() {}

void loop() {
  current_time = rtc.now();
  update_left = false;
  update_right = false;

  if (current_mode == kModeClock) {
    if (last_time.hour() != current_time.hour()) {
      update_left = true;
    }
    if (last_time.minute() != current_time.minute()) {
      update_right = true;
    }
    display.writeBufferTime(militaryToStandard(current_time.hour()),
                            current_time.minute());
    display.displayBuffer(update_left, update_right);
  }
  last_time = current_time;
  delay(500);
}

uint8_t militaryToStandard(uint8_t hour) {
  hour = hour % 12;
  if (hour == 0) hour = 12;
  return hour;
}

void printDisplay() {
  Segments* buffer = display.readDisplay();
  Serial.print(buffer[0], HEX);
  Serial.print(" ");
  Serial.print(buffer[1], HEX);
  Serial.print(" : ");
  Serial.print(buffer[2], HEX);
  Serial.print(" ");
  Serial.println(buffer[3], HEX);
}

// // Sets hour/minute digits to blank until it's light out
// void blackOut(bool h, bool m) {
//   for (int i=0; i<4; i++) {
//     shiftDigit(10,0);
//   }
//   flipLatch();
//   switchHM(h,m);
//   while (analogRead(A0) > LIGHT_TRIGGER) {
//     delay(100);
//   }
//   shiftTime(current);
//   latchIn();
// }

//   // if (!isMilitaryTime) {
//   //   hour = (hour + 11) % 12 + 1; // Convert from 0-24 to 1-12
//   // }
// void loop() {
//   current = rtc.now();                           // Update the time
//   int light_reading = analogRead(A0);
//   Serial.println(light_reading);
//   if (light_reading > LIGHT_TRIGGER) {          // If it's dark out turn off
//     blackOut(1,1);
//   } else {
//     if (last.minute() != current.minute()) {     // If the time has changed, update!
//       shiftTime(current);
//       latchIn();
//     } else {
//       if (mode == 1) {
//         last = current;
//         blackOut(0,1);
//         while (mode == 1) {
//           //set minutes
//           if (digitalRead(upPin) == LOW) current = DateTime(current.unixtime()+60);
//           if (digitalRead(downPin) == LOW) current = DateTime(current.unixtime()-60);
//           if (last.unixtime() != current.unixtime()) {
//             rtc.adjust(current);
//             shiftTime(current);
//             latchIn();
//             last = current;
//           }
//         }
//         blackOut(1,0);
//         while (mode == 2) {
//           //set hours
//           if (digitalRead(upPin) == LOW) current = DateTime(current.unixtime()+3600);
//           if (digitalRead(downPin) == LOW) current = DateTime(current.unixtime()-3600);
//           if (last.unixtime() != current.unixtime()) {
//             rtc.adjust(current);
//             shiftTime(current);
//             latchIn();
//             last = current;
//           }
//         }
//         mode=0;
//         blackOut(1,1);
//       }
//     }
//   }
//   last = current;    // Save the time for next time
//   delay(500);        // Only do this every half second
// }
