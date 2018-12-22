/**
 * This sketch recieves a message from the Master when rain is detected
 * and starts a cascade on the NEOPIXELS to announce rain.
 * It is always on and awaiting messages from the Master
 * The sketch is based on work by Andreas Spiess and Anthony Elder
 * It uses ESPNow to talk
 * The light effect is from:https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 * A great site for most of my Neopixel effects...it is the last: Meteor Fall
 * 
 
 */
#include <ESP8266WiFi.h>
#include "FastLED.h"
#define NUM_LEDS 86
CRGB leds[NUM_LEDS];
#define PIN D7 
float timerOne = 0;
extern "C" {
  #include "user_interface.h"
  #include <espnow.h>
}


//-------- Customise the above values --------
#define SENDTOPIC "ESPNow/key"
#define COMMANDTOPIC "ESPNow/command"
#define SERVICETOPIC "ESPNow/service"

/* Set a private Mac Address
 *  http://serverfault.com/questions/40712/what-range-of-mac-addresses-can-i-safely-use-for-my-virtual-machines
 * Note: the point of setting a specific MAC is so you can replace this Gateway ESP8266 device with a new one
 * and the new gateway will still pick up the remote sensors which are still sending to the old MAC 
 */
uint8_t mac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
void initVariant() {
  
}

IPAddress server(192, 168, 0, 203);

// the X's get replaced with the remote sensor device mac address
const char deviceTopic[] = "ESPNOW/";



String deviceMac;

// keep in sync with ESP_NOW sensor struct
struct __attribute__((packed)) SENSOR_DATA {
   char testdata[240];
} sensorData;

volatile boolean haveReading = false;


int heartBeat;


void setup() {
  Serial.begin(115200); 
  Serial.println();
  Serial.println();
  Serial.println("ESP_Now Controller");
    Serial.println();
FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  WiFi.mode(WIFI_AP);
  Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("This node STA mac: "); Serial.println(WiFi.macAddress());

  initEspNow();  
  Serial.println("Setup done");
}


void loop() {
  if (millis()-heartBeat > 30000) {
    Serial.println("Waiting for ESP-NOW messages...");
    heartBeat = millis();
  }

  if (haveReading) {
    timerOne = millis();
    /* The master sends updates depending on how you adjust the Hardware
     *  timer.  I set mine for once every ten minutes.  The rain tube turns on for 
     *  the period of the while loop below.  If you want it to drip for the whole
     *  time between reading just adjust the time function. You can also vary the 
     *  color of the drip, and various timing parameters here.
     */
    while(millis()-timerOne<60000){
      int q=random(20,60);
      int b = random( 64,150);
      int p = random(5,10);
      int r = random(100, 2000);
      meteorRain(0x1C,0x69,0x24,p, b, true, q);
      delay(r);
    }
    setAll(0,0,0);
    haveReading = false;
    Serial.println("I have data!");
    
  }
}


void initEspNow() {
  if (esp_now_init()!=0) {
    Serial.println("*** ESP_Now init failed");
    ESP.restart();
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {

    deviceMac = "";
    deviceMac += String(mac[0], HEX);
    deviceMac += String(mac[1], HEX);
    deviceMac += String(mac[2], HEX);
    deviceMac += String(mac[3], HEX);
    deviceMac += String(mac[4], HEX);
    deviceMac += String(mac[5], HEX);
    
    memcpy(&sensorData, data, sizeof(sensorData));
//this is where you would find the recieved data if you want to measure 
// battery level or temp or some other info from your slave
    Serial.print("Message received from device: "); Serial.print(deviceMac);
    Serial.println(sensorData.testdata);
    Serial.println();

    haveReading = true;
  });
}
void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
  setAll(30,0,0);
  for(int i = NUM_LEDS; i > 0; i--) {
    
    
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      } 
    }
   
    showStrip();
    delay(SpeedDelay);
  }
}

void fadeToBlack(int ledNo, byte fadeValue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
    
    oldColor = strip.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
    
    strip.setPixelColor(ledNo, r,g,b);
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[ledNo].fadeToBlackBy( fadeValue );
 #endif  
}
 
// ---> here we define the effect function <---
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


