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

// allow us to use itoa() in this scope
extern char* itoa(int a, char* buffer, unsigned char radix);

using namespace lmx;

// Create 3 IntervalTimer objects
IntervalTimer myTimer;
CLedMatrix g_Display;

int g_Seconds = 321;

void RefreshDisplayTimerCallback(void);
void SecTickUpate(void);

int WriteText(String);

void setup(void) {
    g_Display.Initialize();
    //g_Display.PutText("Testing abcdefg");
    char buf[10];
    itoa(g_Seconds, buf, 10);
    g_Display.PutText(buf);
    myTimer.begin(RefreshDisplayTimerCallback, RefreshPeriodUs - 1, uSec);
    //myTimer.begin(SecTickUpate, 1000, hmSec);
    Spark.function("WriteText", WriteText);
    Spark.variable("secs", &g_Seconds, INT);
    /*
    httpClient = new HttpClient();
    weather = new Weather("London,UK", httpClient,
            "INSERT your api key here!");
    weather->setCelsius();
    */
}

// The main program will print the blink count
// to the Arduino Serial Monitor
void loop(void) {
    static int lastVal = 0;
    char buf[10];
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
  /*
    weather_response_t resp = weather->cachedUpdate();
    if (resp.isSuccess)
    {
        WriteText
    }*/
    g_Seconds = millis() / 1000;
    if(g_Seconds != lastVal)
    {
        itoa(g_Seconds, buf, 10);
        WriteText(String(buf));
        lastVal = g_Seconds;
    }

    g_Display.BackgroundProc();

}
/*
void SecTickUpate()
{
    char buf[10];
    //itoa(g_Seconds, buf, 10);
    WriteText(String("ticks = "));// + String(buf));
    g_Seconds++;
}
*/


int WriteText(String Text)
{
    int32_t cols;
    //g_Display.Scroll(g_Display.NUMBER_OF_COLUMNS);
    g_Display.FlushBuffers();
    cols = g_Display.PutText(Text);
    return cols;
}


void RefreshDisplayTimerCallback()
{
    g_Display.Refresh();
}
