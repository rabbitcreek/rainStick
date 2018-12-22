/*
 * 
 * This sketch sends a ESPNow message to an awaiting Rain Stick when rain hits the sensor--it brings 
 * the sensor to LOW on RainPin 14 when completed it sends a HIGH/LOW signal to the adafruit timer circuit
 * to turn off the device.  If it senses no rain no message is sent and it shuts the timer down.  The sleep
 * timer is hardware set on the Adafruit unit by adjusting the potentiometer for anywhere up to 2 hours.  
 * The program is a riff on Andreas Spiess and Anthony Elders work on ESPNow.  
  
*/
#include <ESP8266WiFi.h>
extern "C" {
#include <espnow.h>
}


// this is the MAC Address of the remote ESP server which receives these sensor readings
// Yea this is the most important part of this program--get this address right!
uint8_t remoteMac[] = {0x2E, 0x3A, 0xE8, 0x0E, 0xCD, 0x6B};

#define WIFI_CHANNEL 1
#define DONEPIN 12  //signal pin to turn off Adafruit Timer
#define RainPin 14 //signal pin from rain sensor--I used the digital output

#define MESSAGELEN 10

unsigned long entry;

// keep in sync with slave struct
struct __attribute__((packed)) SENSOR_DATA {
  char testdata[MESSAGELEN];
} sensorData;

//BME280 bme280;
int rainNow;
volatile boolean callbackCalled;

unsigned long entry1 = millis();

void setup() {
  int i = 0;
  pinMode(DONEPIN, OUTPUT);
  digitalWrite(DONEPIN, LOW);
  pinMode(RainPin, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("ESP_Now Controller");
    Serial.println();
    delay(500);
     rainNow = digitalRead(RainPin);
     Serial.print("rainpin");
     Serial.println(rainNow);
  delay(500);
  //If sensor detects no rain--shut down master asap
    if(rainNow){
       while (1) {
    digitalWrite(DONEPIN, HIGH);
    delay(1);
    digitalWrite(DONEPIN, LOW);
    delay(1);
  }
    }
   
  WiFi.mode(WIFI_STA); // Station mode for esp-now sensor node
  WiFi.disconnect();

  Serial.printf("This mac: %s, ", WiFi.macAddress().c_str());
  Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", remoteMac[0], remoteMac[1], remoteMac[2], remoteMac[3], remoteMac[4], remoteMac[5]);
  Serial.printf(", channel: %i\n", WIFI_CHANNEL);

  if (esp_now_init() != 0) {
    Serial.println("*** ESP_Now init failed");
    Serial.println("set-up failed");
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  Serial.println(millis() - entry1);
  unsigned long entry2 = millis();
  esp_now_add_peer(remoteMac, ESP_NOW_ROLE_SLAVE, WIFI_CHANNEL, NULL, 0);
  Serial.println(millis() - entry2);
  unsigned long entry3 = millis();

  esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {
    Serial.printf("send_cb, send done, status = %i\n", sendStatus);
    callbackCalled = true;
  });
  Serial.println(millis() - entry3);
  unsigned long entry4 = millis();

  callbackCalled = false;
//I left this array to be sent even though I dont use the data--if you want you can fill it
// with info like battery level or wind speed or temp--something else you want to measure from
// your remote station
  for (i = 0; i < MESSAGELEN; i++) sensorData.testdata[i] = '0';
  sensorData.testdata[MESSAGELEN] = '\0';
}

void loop() {
  
rainNow = digitalRead(RainPin);
if(!rainNow){

delay(20);
  Serial.println(rainNow);
  uint8_t bs[sizeof(sensorData)];
  memcpy(bs, &sensorData, sizeof(sensorData));
  unsigned long entry = millis();
  esp_now_send(NULL, bs, sizeof(sensorData)); // NULL means send to all peers
  Serial.print("Time to send: ");
  Serial.println(millis() - entry);
  Serial.print("Overall Time: ");
  Serial.println(millis() - entry1);
  //  Serial.print("Size: ");
  //  Serial.println(sizeof(bs));
}
// turn off master once information is sent
  while (1) {
    digitalWrite(DONEPIN, HIGH);
    delay(1);
    digitalWrite(DONEPIN, LOW);
    delay(1);
  //delay(10000);
}
}

