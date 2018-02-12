#include <Arduino.h>
#include "SerialClock.h" 
#include "RTClib.h"
#include "EnableInterrupt.h"


/*===============================================================================================*/
#define DEBUG 1

#define debug_start(baud) do { if (DEBUG) Serial.begin(baud); } while (0)
#define debug_print(...) do { if (DEBUG) Serial.println(__VA_ARGS__); } while (0)
/*===============================================================================================*/

/* Output Pin Definitions */
#define N_OUT_PINS 5
// Pins connected to active-low output enable (!OE)
#define LEFTEN_PIN 9
#define RIGHTEN_PIN 10
// Pin connected to strobe (latch)
#define STROBE_PIN 11
// Pin connected to clock
#define CLOCK_PIN 12
// Pin connected to data
#define DATA_PIN 13
uint8_t output_pins[N_OUT_PINS] = {LEFTEN_PIN, RIGHTEN_PIN, STROBE_PIN, CLOCK_PIN, DATA_PIN};

/* Input Pin Definitions */
#define N_IN_PINS 6
// Buttons
#define CLOCK_MODE_PIN 2
#define COUNT_MODE_PIN 3
#define CTRL1_PIN 4
#define CTRL2_PIN 5
#define UP_PIN 6
#define DOWN_PIN 7
uint8_t input_pins[N_IN_PINS] = {CLOCK_MODE_PIN, COUNT_MODE_PIN, CTRL1_PIN, CTRL2_PIN, UP_PIN, DOWN_PIN};

#define LIGHT_TRIGGER 100

// Operating modes
typedef enum {
  MODE_CLOCK,
  MODE_CLOCK_SET_HR,
  MODE_CLOCK_SET_MIN,
  MODE_STOPWATCH_SET,
  MODE_STOPWATCH_RUN,
  MODE_STOPWATCH_PAUSE,
  MODE_TIMER_SET,
  MODE_TIMER_RUN,
  MODE_TIMER_PAUSE
} Modes;

/*===============================================================================================*/
DateTime last, current;
RTC_DS1307 rtc;

// Current mode
volatile Modes current_mode;

/*===============================================================================================*/
void setup() {
  debug_start(9600);

  // Set up output pins
  for (uint8_t i = 0; i < N_OUT_PINS; i++) {
    pinMode(output_pins[i], OUTPUT);
  }

  // Set up input pins
  for (uint8_t i = 0; i < N_IN_PINS; i++) {
    pinMode(input_pins[i], INPUT_PULLUP);
    enableInterrupt(input_pins[i] | PINCHANGEINTERRUPT, buttonHandler, FALLING);
  }

  rtc.begin(); // Always returns true
  if (!rtc.isrunning()) {
    debug_print("RTC is NOT running!");
    while(1);
  }
  // Set the RTC to the date & time this sketch was compiled:
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void buttonHandler() {
}

void loop() {
  debug_print("idle");
  delay(500);
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

// void setTimeMode() {
//   switch (mode) {
//     case 0:
//       mode = 1;
//       break;
//     case 1:
//       mode = 2;
//       break;
//     default:
//       mode = 0;
//       break;
//   }
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
//   last = current;                                // Save the time for next time
//   delay(500);                                    // Only do this every half second
// }
