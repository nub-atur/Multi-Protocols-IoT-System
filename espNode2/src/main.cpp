#include <Arduino.h>
#include "DHT.h"
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <LiquidCrystal_I2C.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DHTPIN 25 
#define obsLED 12
#define newConnectLED 14
#define DHTTYPE DHT22
#define MESH_PREFIX    "RNTMESH"      //name for your MESH
#define MESH_PASSWORD  "MESHpassword" //password for your MESH
#define MESH_PORT      5555           //default port

struct dataDHT {
  volatile double temp;
  volatile double hum;
};

volatile boolean onReceiveFlag = false; //checking flag for receving from LoRa
String readings;                        //String to send to other nodes with sensor readings
int lcdColumns = 16;
int lcdRows = 2;
// int nodeNumber = 2;                  //Number for this node

LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows);
DHT dht(DHTPIN, DHTTYPE);

Scheduler userScheduler;               // to control your personal task
painlessMesh mesh;

// User stub
void sendMessage() ;                  // Prototype so PlatformIO doesn't complain
String getReadings();                 // Prototype for sending sensor readings
void receivedCallback( uint32_t from, String &msg );
dataDHT readDHT(void);
void LCD_log(void);
// void vRead(void *pvParameters);
void vLCDlog(void *pvParameters);

//-----------------------------------Queue-------------------------------------------------
QueueHandle_t qCMD;

//------------------------------------TASKS-------------------------------------------------
//Create tasks: to send messages and get readings;
Task taskSendMessage(TASK_SECOND , TASK_FOREVER, &sendMessage);
// TaskHandle_t Read;
TaskHandle_t LcdLog;

// void vRead(void *pvParameters){
//   (void)pvParameters;

//   pinMode(newConnectLED,OUTPUT);
//   dht.begin();
//   for (;;){
//     readDHT();
//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

void vLcdLog(void *pvParameters){
  (void)pvParameters;

  pinMode(obsLED,OUTPUT);   
  for (;;){
    LCD_log();
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

//-----------------------------------functions-----------------------------------------
String getReadings () {
  dataDHT data = readDHT();
  JSONVar jsonReadings;
  jsonReadings["node"] = 2;
  jsonReadings["temp"] = data.temp;
  jsonReadings["hum"] = data.hum;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void LCD_log(void){
  char data[32];
  // uint8_t count = 0;
  if (onReceiveFlag == true){
    if(qCMD != NULL){
      int ret = xQueueReceive(qCMD, &data, 50);
      if(ret == pdPASS){
        lcd.clear();
        for (uint8_t i=0; i<32; i++){
          if (!isalpha(data[i]) && (data[i]) != 32) break;
          if (i<16){
            lcd.setCursor(i,0);
          } else {
            lcd.setCursor(i-16,1);
          }
          lcd.print(data[i]);
        } 
      } else if(ret == pdFALSE){
        // Serial.println("Unable to receive data from the Queue");
      }
      memset(data, 0, sizeof(data));
    }
    onReceiveFlag = false;
  }
  vTaskDelay(pdMS_TO_TICKS(2000));
}

dataDHT readDHT(void){
  dataDHT data;
  data.temp = dht.readTemperature();
  data.hum = dht.readHumidity();
  return data;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  JSONVar myObject = JSON.parse(msg.c_str());
  if (uint8_t(myObject["node"]) == 1)
  {
    bool obs = myObject["human"];
    digitalWrite(obsLED, obs ? HIGH : LOW);
  } 
  else if (uint8_t(myObject["node"]) == 3)
  {
    char taking[32] = "";
    strcpy(taking, myObject["cmd"]);
    if (taking[0] != 'n' && taking[1] != 'u' && taking[2] != 'l' && taking[3] != 'l'){

      digitalWrite(newConnectLED, !digitalRead(newConnectLED));
      onReceiveFlag = true;

      if(qCMD != NULL && uxQueueSpacesAvailable(qCMD) > 0){
        int ret = xQueueSend(qCMD, (void*) &taking, 0);
        if(ret == pdTRUE){
          // The message was successfully sent.
        }else if(ret == errQUEUE_FULL){
          // Since we are checking uxQueueSpacesAvailable this should not occur, however if more than one task should
          //   write into the same queue it can fill-up between the test and actual send attempt
          Serial.println("Unable to send data into the Queue");
        } // Queue send check
      }
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
  // Serial.printf("New Connection, nodeId = %u\n", nodeId);
  //vTaskResume(hled);
}

// void changedConnectionCallback() {
//   // Serial.printf("Changed connections\n");
// }

// void nodeTimeAdjustedCallback(int32_t offset) {
//   Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
// }

void setup() {
  //--------------------------------------------------------------------------------
  lcd.init();       
  lcd.backlight(); // turn on LCD backlight 
  Serial.begin(115200);
  // pinMode(LED_BUILTIN, INPUT);
  pinMode(newConnectLED,OUTPUT);
  dht.begin();
  //--------------------------------------------------------------------------------

  //------------------------------MESH-----------------------------------------------
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  // mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  //--------------------------------------------------------------------------------
  
  //---------------------------task test--------------------------------------------
  // xTaskCreatePinnedToCore(
  //   vLEDFlashTask, "LEDTask" // A name just for humans
  //   ,
  //   1024 // This stack size can be checked & adjusted by reading the Stack Highwater
  //   ,
  //   NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
  //   ,
  //   &hled //handle
  //   ,
  //   0);  //core 1
  // //-----------------------------------------------------------------------------

  qCMD = xQueueCreate(5, sizeof(char[32]));
  xTaskCreatePinnedToCore(vLcdLog, "log", 1024*5, NULL, 0, &LcdLog, 0);
  // xTaskCreate(vRead, "ReadDHT", 1024*2, NULL, 1, &Read);
}

void loop() {
  mesh.update();
}

