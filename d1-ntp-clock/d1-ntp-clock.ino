//
//   NTP-Based Clock
//
//   This is a simple implementation of an NTP clock with NeoPixel lights
//   Based on the NTP Clock by Steve https://steve.fi/Hardware/
//
//   Dave
//   --
//


//
// WiFi & over the air updates
//
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>

#define PIN 12
#define NUM_LEDS 8

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// Clock status
// 0 setup
// 1 normal
// 2 pre-bed-time
// 3 sleep
// 4 wake-up
int CLOCK_STATUS = 0;

//
// For dealing with NTP & the clock.
//
#include "NTPClient.h"

//
// The display-interface
//
#include "TM1637.h"


//
// WiFi setup.
//
#include "WiFiManager.h"


//
// Debug messages over the serial console.
//
#include "debug.h"


//
// The name of this project.
//
// Used for:
//   Access-Point name, in config-mode
//   OTA name.
//
#define PROJECT_NAME "NTP-CLOCK"


//
// The timezone
// GMT is +0, +1 for DST
//
#define TIME_ZONE (+1)


//
// NTP client, and UDP socket it uses.
//
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


//
// Pin definitions for TM1637 and can be changed to other ports
//
#define CLK D3
#define DIO D2
TM1637 tm1637(CLK, DIO);


//
// Called just before the date/time is updated via NTP
//
void on_before_ntp()
{
  DEBUG_LOG("Updating date & time\n");
}

//
// Called just after the date/time is updated via NTP
//
void on_after_ntp()
{
  DEBUG_LOG("Updated NTP client\n");
  
  if(CLOCK_STATUS == 0)
  {
    // set the strip to green
    FadeIn(0x00, 0xff, 0x00);
    FadeInOut(0x00, 0xff, 0x00);
    FadeInOut(0x00, 0xff, 0x00);
  }
  
  CLOCK_STATUS = 1;
}

//
// This function is called when the device is powered-on.
//
void setup()
{
  //Initialise the lights
  strip.begin();
  strip.show();

  // Enable our serial port.
  Serial.begin(115200);
  
  // set the strip to red
  FadeIn(0xff, 0x00, 0x00);

  //
  // Handle WiFi setup
  //
  WiFiManager wifiManager;
  wifiManager.autoConnect(PROJECT_NAME);

  //
  // Ensure our NTP-client is ready.
  //
  timeClient.begin();

  
  // set the strip to yellow
  FadeIn(0xff, 0xe6, 0x00);

  //
  // Configure the callbacks.
  //
  timeClient.on_before_update(on_before_ntp);
  timeClient.on_after_update(on_after_ntp);

  //
  // Setup the timezone & update-interval.
  //
  timeClient.setTimeOffset(TIME_ZONE * (60 * 60));
  timeClient.setUpdateInterval(300 * 1000);


  //
  // The final step is to allow over the air updates
  //
  // This is documented here:
  //     https://randomnerdtutorials.com/esp8266-ota-updates-with-arduino-ide-over-the-air/
  //
  // Hostname defaults to esp8266-[ChipID]
  //
  ArduinoOTA.setHostname(PROJECT_NAME);

  ArduinoOTA.onStart([]()
  {
    DEBUG_LOG("OTA Start\n");
  });
  ArduinoOTA.onEnd([]()
  {
    DEBUG_LOG("OTA End\n");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    char buf[32];
    memset(buf, '\0', sizeof(buf));
    snprintf(buf, sizeof(buf) - 1, "Upgrade - %02u%%\n", (progress / (total / 100)));
    DEBUG_LOG(buf);
  });
  ArduinoOTA.onError([](ota_error_t error)
  {
    DEBUG_LOG("Error - ");

    if (error == OTA_AUTH_ERROR)
      DEBUG_LOG("Auth Failed\n");
    else if (error == OTA_BEGIN_ERROR)
      DEBUG_LOG("Begin Failed\n");
    else if (error == OTA_CONNECT_ERROR)
      DEBUG_LOG("Connect Failed\n");
    else if (error == OTA_RECEIVE_ERROR)
      DEBUG_LOG("Receive Failed\n");
    else if (error == OTA_END_ERROR)
      DEBUG_LOG("End Failed\n");
  });

  //
  // Ensure the OTA process is running & listening.
  //
  ArduinoOTA.begin();
}

//
// This function is called continously, and is responsible
// for flashing the ":", and otherwise updating the display.
//
// We rely on the background NTP-updates to actually make sure
// that that works.
//
void loop()
{
  static char buf[10] = { '\0' };
  static char prev[10] = { '\0' };
  static long last_read = 0;

  //
  // Resync the clock?
  //
  timeClient.update();

  //
  // Handle any pending over the air updates.
  //
  ArduinoOTA.handle();

  //
  // Get the current hour/min
  //
  int cur_hour = timeClient.getHours();
  int cur_min  = timeClient.getMinutes();

  //DEBUG_LOG(String(cur_hour).c_str());
  //DEBUG_LOG("\n");

  //DEBUG_LOG(timeClient.getFormattedTime().c_str());
  //DEBUG_LOG("\n");

  if(CLOCK_STATUS > 0)
  {
      
  
  
  /*
    switch(CLOCK_STATUS)
    {
      case 0: // 0 setup
        break;
      case 1: // 1 normal
        SetColor(0x7a, 0x7a, 0x7a);
        break;
      case 2: // 2 pre-bed-time
        SetColor(0xff, 0x00, 0x00);
        break;
      case 3: // 3 sleep
        SetColor(0xff, 0x00, 0xff);
        break;
      case 4: // 4 wake-up
        RunningLights(0xff,0xff,0x00, 50);
        break;
    }
  */
  
    if(cur_hour == 6)
    {
      if(cur_min >= 30)
      {
        RunningLights(0xff,0xff,0x00, 50);
        //SetColor(0xff, 0xe6, 0x00);
      }
    }
    else if(cur_hour == 7)
    {
      SetColor(0x00, 0xff, 0x00);
    }
    else if(cur_hour == 8)
    {
      SetColor(0x00, 0x00, 0x00);
    }
    else if(cur_hour == 18)
    {
      SetColor(0x00, 0x00, 0xff);
    }
    else if(cur_hour == 19)
    {
      SetColor(0xff, 0x00, 0x00);
    }
    else if(cur_hour > 19 || cur_hour < 6)
    {
      SetColor(0xff, 0x00, 0x00);
    }
    else
    {
      SetColor(0x00, 0x00, 0x00);
    }
  }

/*
  if(cur_hour == 6 && cur_min == 0)
  {
    RunningLights(0xff,0xff,0x00, 50);
  }
  else if(cur_hour == 6 && cur_min == 30)
  {
    SetColor(0xff, 0xe6, 0x00);
  } 
  else if(cur_hour == 8 && cur_min == 0)
  {
    SetColor(0x00, 0x00, 0x00);
  }
  else if(cur_hour == 18 && cur_min == 0)
  {
    SetColor(0x00, 0x00, 0xff);
  }
  else if(cur_hour == 19 && cur_min == 0)
  {
    SetColor(0xff, 0x00, 0x00);
  }
  else{
    SetColor(0x00, 0x00, 0x00);
    //RunningLights(0xff,0xff,0x00, 50);
  }
*/

  //
  // Format them in a useful way.
  //
  //sprintf(buf, "%02d%02d", cur_hour, cur_min);

  //
  // If the current "hourmin" is different to
  // that we displayed last loop ..
  //
  //if (strcmp(buf, prev) != 0)
  //{
    // Update the display
    //tm1637.display(0, buf[0] - '0');
    //tm1637.display(1, buf[1] - '0');
    //tm1637.display(2, buf[2] - '0');
    //tm1637.display(3, buf[3] - '0');

    // And cache it
    //strcpy(prev , buf);
  //}

  //
  // The preceeding piece of code would
  // have ensured the display only updated
  // when the hour/min changed.
  //
  // However note that we nuke the cached
  // value every half-second - solely so we can
  // blink the ":".
  //
  //  Sigh

  //long now = millis();

  //if ((last_read == 0) ||
   //   (abs(now - last_read) > 500))
  //{
    // Invert the "show :" flag
    //flash = !flash;

    // Apply it.
    //tm1637.point(flash);

    //
    // Note that the ":" won't redraw unless/until you update.
    // So we'll force that to happen by removing the cached
    // value here.
    //
    //memset(prev, '\0', sizeof(prev));
    //last_read = now;
  //}
}

void SetColor(byte red, byte green, byte blue){
  float r, g, b;
  int k=255;
  r = (k/256.0)*red;
  g = (k/256.0)*green;
  b = (k/256.0)*blue;
  setAll(r,g,b);
  showStrip();
}

void FadeInOut(byte red, byte green, byte blue){
  float r, g, b;
      
  for(int k = 0; k < 256; k=k+1) { 
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    showStrip();
  }
     
  for(int k = 255; k >= 0; k=k-2) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    showStrip();
  }
}

void FadeIn(byte red, byte green, byte blue){
  float r, g, b;
      
  for(int k = 0; k < 256; k=k+1) { 
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    showStrip();
  }
}

void FadeOut(byte red, byte green, byte blue){
  float r, g, b;
     
  for(int k = 255; k >= 0; k=k-2) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    showStrip();
  }
}

// *** REPLACE TO HERE ***

void showStrip() {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position=0;
  
  for(int i=0; i<NUM_LEDS*2; i++)
  {
      Position++; // = 0; //Position + Rate;
      for(int i=0; i<NUM_LEDS; i++) {
        // sine wave, 3 offset waves make a rainbow!
        //float level = sin(i+Position) * 127 + 128;
        //setPixel(i,level,0,0);
        //float level = sin(i+Position) * 127 + 128;
        setPixel(i,((sin(i+Position) * 127 + 128)/255)*red,
                   ((sin(i+Position) * 127 + 128)/255)*green,
                   ((sin(i+Position) * 127 + 128)/255)*blue);
      }
      
      showStrip();
      delay(WaveDelay);
  }
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif
}

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}
