// Spark Interval Timer demo
//
// This demo will create three Interval Timers (maximum allowed) to blink three
// LEDs at different intervals.  The first timer will blink the onboard LED
// while 2 extra LEDs (and small current limiting resistors) must be added
// by the user on pins D3 and D4.  After 100 blinks, Timer1 will reset to 1/4
// of its interval and after 200 more blinks, Timer1 is shut down and
// will stop blinking.


#include "SparkIntervalTimer.h"
#include "LedMatrix.h"

using namespace lmx;

// Create 3 IntervalTimer objects
IntervalTimer myTimer;
CLedMatrix g_Display;


// Pre-define ISR callback functions
void RefreshDisplayTimerCallback(void);

void setup(void) {
    g_Display.Initialize();
    myTimer.begin(RefreshDisplayTimerCallback, RefreshPeriodUs - 1, uSec);
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

void RefreshDisplayTimerCallback()
{
    g_Display.Refresh();
}
