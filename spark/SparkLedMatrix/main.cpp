// Spark Interval Timer demo
//
// This demo will create three Interval Timers (maximum allowed) to blink three
// LEDs at different intervals.  The first timer will blink the onboard LED
// while 2 extra LEDs (and small current limiting resistors) must be added
// by the user on pins D3 and D4.  After 100 blinks, Timer1 will reset to 1/4
// of its interval and after 200 more blinks, Timer1 is shut down and
// will stop blinking.


#include "SparkIntervalTimer.h"

#define NUMBER_OF_BOARDS 4
#define COLUMNS_PER_BOARD 15

// Create 3 IntervalTimer objects
IntervalTimer myTimer;

// Pre-define ISR callback functions
void refreshDisplay(void);

const uint8_t ledPin = D7;		// LED for first Interval Timer
const uint8_t rclkPin = D2;

void setup(void) {
  pinMode(ledPin, OUTPUT);

  digitalWrite(rclkPin, LOW);
  pinMode(rclkPin, OUTPUT);

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE0);

  myTimer.begin(refreshDisplay, 1000, uSec);
}

int ledState = LOW;

// Callback for Timer 1
void refreshDisplay(void) {
  static uint16_t activeColIndex = NUMBER_OF_BOARDS;

  if (ledState == LOW) {
    ledState = HIGH;
	PIN_MAP[ledPin].gpio_peripheral->BSRR = PIN_MAP[ledPin].gpio_pin; // LED High
  }
  else {
    ledState = LOW;
    PIN_MAP[ledPin].gpio_peripheral->BRR = PIN_MAP[ledPin].gpio_pin; // LED low
  }

  for(int8_t i = (NUMBER_OF_BOARDS - 1); i >= 0; i--)
  {
    SPI.transfer((1 << activeColIndex) >> 8);
    SPI.transfer((1 << activeColIndex) & 0xFF);
    SPI.transfer((activeColIndex << 1) & 0xFF);
  }

  digitalWrite(rclkPin, HIGH);
  //delayMicroseconds(1);
  digitalWrite(rclkPin, LOW);

  if(activeColIndex == 0)
    activeColIndex = COLUMNS_PER_BOARD - 1;
  else
    activeColIndex--;
}

// The main program will print the blink count
// to the Arduino Serial Monitor
void loop(void) {
//  unsigned long blinkCopy;  // holds a copy of the blinkCount

  // to read a variable which the interrupt code writes, we
  // must temporarily disable interrupts, to be sure it will
  // not change while we are reading.  To minimize the time
  // with interrupts off, just quickly make a copy, and then
  // use the copy while allowing the interrupt to keep working.
//  noInterrupts();
//  blinkCopy = blinkCount;
//  interrupts();
/*
  if (blinkCount == 100)	{			// After 100 blinks, shut down timer 1
    blinkCount++;						// increment count so IF does not keep passing
	myTimer.resetPeriod_SIT(250, hmSec);
	}
  else if (blinkCount >= 300) {			// After 100 blinks, shut down timer 1
	blinkCount = 0;						// reset count so IF does not keep passing
	myTimer.end();

	}
  */


}
