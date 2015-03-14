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
bool g_WeatherRequestPending = false;

String tryExtractString(String str, const char* start, const char* end);
void gotWeatherData(const char *name, const char *data);
void RefreshDisplayTimerCallback(void);
void SecTickUpate(void);

int WriteText(String);

void setup(void) {
    Serial.begin(115200);
    Serial.println("startup");
    g_Display.Initialize();
    //g_Display.PutText("Testing abcdefg");
    char buf[10];
    itoa(g_Seconds, buf, 10);
    g_Display.PutText(buf);
    myTimer.begin(RefreshDisplayTimerCallback, RefreshPeriodUs - 1, uSec);
    //myTimer.begin(SecTickUpate, 1000, hmSec);
    Spark.function("WriteText", WriteText);
    Spark.variable("secs", &g_Seconds, INT);
    Spark.subscribe("hook-response/get_w", gotWeatherData, MY_DEVICES);
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
        Serial.println(buf);
        WriteText(String(buf));
        lastVal = g_Seconds;
    }

    if(g_Seconds % 10 == 0 && !g_WeatherRequestPending)
    {
        // Let's request the weather, but no more than once every 60 seconds.
        Serial.println("Requesting Weather!");

        // publish the event that will trigger our webhook
        Spark.publish("get_w");
        g_WeatherRequestPending = true;
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

// This function will get called when weather data comes in
void gotWeatherData(const char *name, const char *data) {
    // Important note!  -- Right now the response comes in 512 byte chunks.
    //  This code assumes we're getting the response in large chunks, and this
    //  assumption breaks down if a line happens to be split across response chunks.
    //
    // Sample data:
    //  <location>Minneapolis, Minneapolis-St. Paul International Airport, MN</location>
    //  <weather>Overcast</weather>
    //  <temperature_string>26.0 F (-3.3 C)</temperature_string>
    //  <temp_f>26.0</temp_f>


    String str = String(data);
    String locationStr = tryExtractString(str, "<location>", "</location>");
    String weatherStr = tryExtractString(str, "<weather>", "</weather>");
    String tempStr = tryExtractString(str, "<temp_f>", "</temp_f>");
    String windStr = tryExtractString(str, "<wind_string>", "</wind_string>");
    Serial.println("Weather data received!");
    if (locationStr != NULL) {
        Serial.println("At location: " + locationStr);
    }

    if (weatherStr != NULL) {
        Serial.println("The weather is: " + weatherStr);
    }

    if (tempStr != NULL) {
        Serial.println("The temp is: " + tempStr + String(" *F"));
    }

    if (windStr != NULL) {
        Serial.println("The wind is: " + windStr);
    }
    g_WeatherRequestPending = false;
}

// Returns any text found between a start and end string inside 'str'
// example: startfooend  -> returns foo
String tryExtractString(String str, const char* start, const char* end) {
    if (str == NULL) {
        return NULL;
    }

    int idx = str.indexOf(start);
    if (idx < 0) {
        return NULL;
    }

    int endIdx = str.indexOf(end);
    if (endIdx < 0) {
        return NULL;
    }

    return str.substring(idx + strlen(start), endIdx);
}
