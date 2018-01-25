/*
PIN ORDER
dat
clk
str
oe
*/

#include <Wire.h>
#include "RTClib.h"

#define LIGHT_TRIGGER 850

DateTime last, current;
RTC_DS1307 rtc;

//Pins connected to !OE
int minuteOE = 13;
int hourOE = 9;
//Pin connected to strobe
int latchPin = 10;
////Pin connected to data
int dataPin = 11;
//Pin connected to clock
int clockPin = 12;

int setPin = 2;
int upPin = 3;
int downPin = 4;

// Mode 0 = Normal Operation
// Mode 1 = Set minutes
// Mode 2 = Set hours
// Mode 3/4 = Set Alarm
volatile int mode = 0;


byte segs[11] = {
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
  B00000000, // [blank]
};

word upDownBytes(byte segs) {
  byte nsegs = ~segs;
  word result = 0;
  for(int i = 15; i >= 0; i--) {
    result |= (segs >> i) & 1;
    result <<= 1;
    result |= (nsegs >> i) & 1;
    if (i != 0) {
      result <<= 1;
    }
  }
  Serial.println(segs, BIN);
  Serial.println(result, BIN);
  return result;
}

void setup() {
  Serial.begin(9600);
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(minuteOE, OUTPUT);
  pinMode(hourOE, OUTPUT);
  pinMode(setPin, INPUT_PULLUP);
  pinMode(upPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  digitalWrite(latchPin, LOW);
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
void blackOut(boolean h, boolean m) {
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

void loop() {
  current = rtc.now();                           // Update the time
  if (analogRead(A0) > LIGHT_TRIGGER) {          // If it's dark out turn off
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
