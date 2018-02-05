#include <Arduino.h>
#include "RTClib.h"
#include "serial_clock.h"

#define LIGHT_TRIGGER 100

DateTime last, current;
RTC_DS1307 rtc;

// Pins connected to !OE
int minuteOE = 9;
int hourOE = 10;
// Pin connected to strobe
int strobePin = 11;
// Pin connected to clock
int clockPin = 12;
// Pin connected to data
int dataPin = 13;


int setPin = 2;
int upPin = 3;
int downPin = 4;

// Mode 0 = Normal Operation
// Mode 1 = Set minutes
// Mode 2 = Set hours
// Mode 3/4 = Set Alarm
volatile int mode = 0;



void setup() {
  Serial.begin(9600);
  
  pinMode(strobePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(minuteOE, OUTPUT);
  pinMode(hourOE, OUTPUT);
  pinMode(setPin, INPUT_PULLUP);
  pinMode(upPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  digitalWrite(strobePin, LOW);
  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);
  digitalWrite(minuteOE, HIGH);
  digitalWrite(hourOE, HIGH);

  attachInterrupt(0, setTimeMode, FALLING);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// Sets hour/minute digits to blank until it's light out
void blackOut(bool h, bool m) {
  for (int i=0; i<4; i++) {
    shiftDigit(10,0);
  }
  flipLatch();
  switchHM(h,m);
  while (analogRead(A0) > LIGHT_TRIGGER) {
    delay(100);
  }
  shiftTime(current);
  latchIn();
}

void setTimeMode() {
  switch (mode) {
    case 0:
      mode = 1;
      break;
    case 1:
      mode = 2;
      break;
    default:
      mode = 0;
      break;
  }
}
  // if (!isMilitaryTime) {
  //   hour = (hour + 11) % 12 + 1; // Convert from 0-24 to 1-12
  // }
void loop() {
  current = rtc.now();                           // Update the time
  int light_reading = analogRead(A0);
  Serial.println(light_reading);
  if (light_reading > LIGHT_TRIGGER) {          // If it's dark out turn off
    blackOut(1,1);
  } else {
    if (last.minute() != current.minute()) {     // If the time has changed, update!
      shiftTime(current);
      latchIn();
    } else {
      if (mode == 1) {
        last = current;
        blackOut(0,1);
        while (mode == 1) {
          //set minutes
          if (digitalRead(upPin) == LOW) current = DateTime(current.unixtime()+60);
          if (digitalRead(downPin) == LOW) current = DateTime(current.unixtime()-60);
          if (last.unixtime() != current.unixtime()) {
            rtc.adjust(current);
            shiftTime(current);
            latchIn();
            last = current;
          }
        }
        blackOut(1,0);
        while (mode == 2) {
          //set hours
          if (digitalRead(upPin) == LOW) current = DateTime(current.unixtime()+3600);
          if (digitalRead(downPin) == LOW) current = DateTime(current.unixtime()-3600);
          if (last.unixtime() != current.unixtime()) {
            rtc.adjust(current);
            shiftTime(current);
            latchIn();
            last = current;
          }
        }
        mode=0;
        blackOut(1,1);
      }
    }
  }
  last = current;                                // Save the time for next time
  delay(500);                                    // Only do this every half second
}
