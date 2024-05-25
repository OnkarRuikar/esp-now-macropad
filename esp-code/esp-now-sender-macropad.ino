/*
  ESP command receiver dongle
  
  The ESP32 device is connected to PC over USB serial port.
  It receives commands over esp-now wifi protocol from
  other ESP32 macro boards. Received commands are sent to 
  pc-serial-macro-server over the serial port.
  
  The receiver dongle also receives commands from PC to be executed
  on its GPIO pins. Commands are to make sounds on buzzer module.
  
  The code assumes the dev board has NeoPixel led. Update the code
  if your board ha different LED.

  created 10th May 2024
  by Onkar Ruikar
*/

#include <IRremote.hpp>
#include <esp_now.h>
#include <WiFi.h>

// update pin number as per your ESP32 board
#define IR_RECEIVE_PIN 15 // D15
#define VOLUME_ADC_PIN 34 // D34
typedef struct struct_message {
    int key;
} struct_message;

struct_message data;
int volume = 0;
int oldVolume = 0;
// use MAC address of dongle ESP32
uint8_t broadcastAddress[] = {0x33, 0xB6, 0xD0, 0x51, 0x9A, 0x9B};
esp_now_peer_info_t peerInfo;

int getVolume() {
  int potValue = 0;
  for (int i = 0; i<40; i++) {
    potValue += analogRead(VOLUME_ADC_PIN);
  }
  potValue /= 40;
  potValue = ((potValue * 100) / 4095);
  return 5000 + potValue;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    //Serial.println("Delivery Success");
    digitalWrite(12, HIGH);
  } else {
    //Serial.println("Delivery Fail");
    digitalWrite(12, LOW);
  }
}

void sendData(int command) {
  data.key = command;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &data, sizeof(data));
  /*if (result == ESP_OK) {
    Serial.println("Sent with success");
    digitalWrite(2, HIGH);
  }
  else {
    Serial.println("Error sending the data");
    digitalWrite(2, LOW);
  }*/
}

void setup()
{
  //Serial.begin(115200);
  delay(2000);
  // connection indicator
  pinMode(12, OUTPUT);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //Serial.println("Failed to add peer");
    sleep(90000000);
    return;
  }

  oldVolume = volume = getVolume();
}

void loop()
{
  if (IrReceiver.decode())
  {
    //Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    //Serial.println(IrReceiver.decodedIRData.command);
    sendData(IrReceiver.decodedIRData.command);
    delay(100);
    IrReceiver.resume();
  }

  volume = getVolume();
  if(abs(volume - oldVolume) > 3) {
    oldVolume = volume;
    sendData(volume);
    //Serial.println(volume);
    delay(200);
  }
}
