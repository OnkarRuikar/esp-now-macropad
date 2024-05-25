/*
  ESP command receiver dongle
  
  The ESP32 device is connected to PC over USB serial port.
  It receives commands over esp-now wifi protocol from
  other ESP32 macro boards. Received commands are sent to 
  pc-serial-macro-server over the serial port.
  
  The receiver dongle also receives commands from PC to be executed
  on its GPIO pins. You can achieve other automations using GPIO pins on
  the dongle ESP32. For now commands are to make sounds on buzzer module.
      
  created 10th May 2024
  by Onkar Ruikar
*/

#include <esp_now.h>
#include <WiFi.h>
#include <BluetoothSerial.h>

#define BUZZER_PIN 6
int inCommand = 0;

typedef struct struct_message {
    int key;
} struct_message;

struct_message data;

/*
 * Handle received command from keypad over esp-now.
*/
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&data, incomingData, sizeof(data));
  neopixelWrite(21, 0, 0, 64);
  
  // convert MAC numer to string
  char cMac[18];
  snprintf(cMac, sizeof(cMac), "%02X-%02X-%02X-%02X-%02X-%02X",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println("got:" + String(cMac) + ":" + String(data.key));
  
  // indicate command sent to PC
  delay(100);
  neopixelWrite(21, 0, 0, 0);
  delay(100);
  neopixelWrite(21, 0, 0, 64);
  delay(100);
  neopixelWrite(21, 5, 0, 0);
}

// Play beep on buzzer
void beep(uint32_t duration) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.begin(115200);
  setCpuFrequencyMhz(80);
  btStop();
  delay(5000);

  // set device as a wifi station
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("log: esp-now init failed!");
    neopixelWrite(21, 0, 5, 0);
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  neopixelWrite(21, 5, 0, 0);
  Serial.println("log: setup done");
}

void loop() {
  // handle commands received from PC
  if (Serial.available()) {
    inCommand = Serial.read();
    Serial.println("log: got command " + String(inCommand));
    switch(inCommand) {
      // beep
      case 49:
        beep(80);
        break;
      // double beep
      case 50: 
        beep(150);
        delay(80);
        beep(200);
      break;
	  // EOD beep
      case 51:
        beep(400);
        delay(100);
        beep(100);
        delay(50);
        beep(100);
        break;
      // buzzer on
      case 52:
        digitalWrite(BUZZER_PIN, HIGH);
        break;
      // buzzer off
      case 53:
        digitalWrite(BUZZER_PIN, LOW);
        break;
      default:
        digitalWrite(BUZZER_PIN, LOW);
    }
    inCommand = 0;
  }
}

// debug scripts

/* ----------------------------------------------
 * script to get MAC address of the device
 * so that it could be used by sender devices
 * to send commands to this device
#include "WiFi.h"
 
void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
}
 
void loop(){}
*/

/* ----------------------------------------------
 * script to test usb serial communication with PC

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup() {
  // initialize serial
  Serial.begin(115200);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop() {
  if (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println("got: " + inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}
*/