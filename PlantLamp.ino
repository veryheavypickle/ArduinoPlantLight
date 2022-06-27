/*
   TimeSerial.pde
   example code illustrating Time library set through serial port messages.

   Messages consist of the letter T followed by ten digit time (as seconds since Jan 1 1970)
   you can send the text on the next line using Serial Monitor to set the clock to noon Jan 1 2013
  T1357041600

   A Processing example sketch to automatically send the messages is included in the download
   On Linux, you can use "date +T%s\n > /dev/ttyACM0" (UTC time zone)
*/

#include <TimeLib.h>
#include <FastLED.h>

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

#define NUM_LEDS 150
#define DATA_PIN 5

CRGB leds[NUM_LEDS];
float Brightness = 0; //brightness of strip
int errorHSVBrightness = 255; //the brightness of the error lights
int duration = 15; //duration for the fade
int fps = 120;
int numOfStrips = 3;
bool timeHasBeenSet = false; //if time has been set, will still remain true if arduino looses track of time

void setup()  {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  setSyncProvider( requestSync);  //set function to call when sync required
  Serial.println("Waiting for sync message");
  FastLED.clear();
}

void loop() {
  if (Serial.available()) {
    processSyncMessage();
  }
  if (timeStatus() != timeNotSet) {
    lightsManager();
    showTime();
  }
  if (timeStatus() == timeSet) {
    // Code if time is synced
    timeHasBeenSet = true;
  } else {
    // Code if time is not synced
    if (!timeHasBeenSet) {
      timeNotSyncedLights();
    }
  }
  delay(1000);
  FastLED.show();
}

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    if ( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      setTime(pctime); // Sync Arduino clock to the time received on the serial port
    }
  }
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);
  return 0; // the time will be sent later in response to serial mesg
}

void lightsManager() {
  int currentHour = hour();
  if (currentHour >= 7 && currentHour < 23) { // if daytime show full daylight
    dayLights();
  } else {
    nightLights();
  }
  Serial.println(currentHour);
}

void dayLights() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
  //fades from black to full white
  if (Brightness != 255) {
    for (int i = 0; i < duration * fps; i++) {
      // maps i with a range of 0 to (fps * duration) to a range of 0 to 255
      Brightness = map(i, 0, duration * fps - 1, 0, 255);
      FastLED.setBrightness(Brightness);
      FastLED.show();
      delay(1 / fps);
    }
  }
}

void nightLights() {
  if (Brightness != 0) {
    for (int i = duration * fps; i > 1; i--) {
      // maps i with a range of 0 to (fps * duration) to a range of 0 to 255
      Brightness = map(i, 0, duration * fps - 1, 0, 255);
      FastLED.setBrightness(Brightness);
      FastLED.show();
      delay(1 / fps);
    }
  } else {
    FastLED.setBrightness(1); // this just allows some patterns to be displayed
  }
  // this is to make sure the animation doesn't freeze on the red error lights on
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void timeNotSyncedLights() {
  // toggle led on and off
  for (int i = 0; i < numOfStrips; i++) {
    leds[(NUM_LEDS / numOfStrips) * i] = CHSV(96, 255, errorHSVBrightness);
    leds[(NUM_LEDS / numOfStrips) * (i + 1) - 1] = CHSV(96, 255, errorHSVBrightness);
  }
  if (errorHSVBrightness == 0) {
    errorHSVBrightness = 255;
  } else {
    errorHSVBrightness = 0;
  }
}

void showTime() {
  // divide the time into single digits
  int displayBorder = 2;
  int tens = minute() / 10;
  int ones = minute() % 10;
  int posOnes = NUM_LEDS / numOfStrips - displayBorder;
  int posTens = posOnes + displayBorder * 2;
  int posHour = posOnes + (NUM_LEDS / numOfStrips) * 2;
  int currentHour = hour();

  if (currentHour > 12) {
    currentHour = currentHour - 12;
  }

  for (int i = ones; i > 0; i--) {
    leds[posOnes - i] = CRGB(255, 0, 255);
  }
  //as the middle strip is reveresed, for statement has to be reversed
  for (int i = 0; i < tens; i++) {
    leds[posTens + i] = CRGB(255, 0, 255);
  }
  for (int i = currentHour; i > 0; i--) {
    leds[posHour - i] = CRGB(255, 0, 255);
  }
}
