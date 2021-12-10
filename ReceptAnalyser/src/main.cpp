#include <Arduino.h>
#include <esp_now.h>
#include "WiFi.h"
#include "ArduinoJson.h"
#include "ESPMQTTClient.h"
//https://randomnerdtutorials.com/esp-now-many-to-one-esp32/
//209aa92e-52f8-4540-ab45-77c37da12a77
#define LED_BUILTIN  2

char data[1024];
int package = 0;
int limit_package = 10;
int Tpack = 5;
int Nboards = 3;
int amostrasLimit=5;
int lastTime = 0;
unsigned long timestep = 20;

typedef struct struct_message {
    int id;
    int temp; // must be unique for each sender board
    int dis;
} struct_message;

struct_message myData;
struct_message board1;
struct_message boardsStruct[1] = {board1};

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].temp = myData.temp;
  boardsStruct[myData.id-1].dis = myData.dis;
  boardsStruct[myData.id-1].id = myData.id;
  Serial.printf("Temperature value: %d \n", boardsStruct[myData.id-1].temp);
  Serial.printf("Distance value   : %d \n", boardsStruct[myData.id-1].dis);
  Serial.println();
}


void init_now(){
  
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
}

void setup() {
  Serial.begin(115200);
  
  init_now();
  esp_now_register_recv_cb(OnDataRecv);

}


void loop() {
  //int lastTimeWD = 0;
  //int timestepWD = 3000;
  StaticJsonDocument<1024> doc;
  Serial.println();
  for (int amostras = 0; amostras <= amostrasLimit; amostras++){
    int auxboard = 1;
    while(auxboard <= Nboards){
      while (true){
        if ((millis() - lastTime) > timestep) {
          if(myData.id == auxboard){
            doc["Temp"][(auxboard-1)][amostras] = boardsStruct[myData.id-1].temp;
            doc["Dist"][(auxboard-1)][amostras] = boardsStruct[myData.id-1].dis;
            auxboard++;
            break;
          }
          lastTime = millis(); 
        }
        // Verificar se um esp foi desconectado e pular para o proximo caso seja verdade
        //if ((millis() - lastTimeWD) > timestepWD ){
        //  doc["Temp"][auxboard][amostras] = nullptr;
        //  doc["Dist"][auxboard][amostras] = nullptr;
        //  auxboard++;
        //  break;
        //}
      }
      //lastTimeWD = millis();
    }
  }
  serializeJson(doc, Serial);
  serializeJson(doc, data);
  serializeJsonPretty(doc, Serial);
  
}