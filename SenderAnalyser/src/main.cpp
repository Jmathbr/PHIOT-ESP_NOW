#include <Arduino.h>

#include <espnow.h>
#include <ESP8266WiFi.h>
//https://randomnerdtutorials.com/esp-now-esp8266-nodemcu-arduino-ide/

uint8_t broadcastAddress[] = {0x94, 0xB9, 0x7E, 0xC3, 0x57, 0x18};

typedef struct struct_message {
    int id;
    int temp; 
    int dis;
} struct_message;


struct_message myData;

unsigned long lastTime = 0;  
unsigned long timestep = 200;  

uint64_t Time_sleep = 5e6;
int verifySucess = 0;
int verifyFailed = 0;
int NTpack = 10;
int Tpack = 5;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    verifySucess++;
    verifyFailed = 0;
    Serial.print("Delivery success: ");
    Serial.println(verifySucess);
  }
  else{
    Serial.print("Delivery fail: ");
    Serial.println(verifyFailed);
    verifyFailed++;
    if(verifyFailed == NTpack){
      ESP.reset();
    } 
  }
}
 
void setup() {
  
  Serial.begin(115200);
  //Serial.setTimeout(2000);
  
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  
}
 
void loop() {
  
  if ((millis() - lastTime) > timestep) {
    myData.id = 3;
    myData.temp = random(1,20);
    myData.dis = random(30,50);
    
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    lastTime = millis();
  }
  if (verifySucess == Tpack){
    verifySucess = 0;
    ESP.deepSleep(Time_sleep);
  }
}