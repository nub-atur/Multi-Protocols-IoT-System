#include <Arduino.h>
#include "DHT.h"
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <LiquidCrystal_I2C.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char packetBuffer[32];
char serverip[] = "192.168.1.4";
unsigned int localPort = 8888;
const char *ssid = "806";
const char *password = "05022001";

#define DHTPIN 25 
#define obsLED 12
#define newConnectLED 14
#define DHTTYPE DHT22
#define MESH_PREFIX    "RNTMESH" //name for your MESH
#define MESH_PASSWORD  "MESHpassword" //password for your MESH
#define MESH_PORT      5555 //default port

struct myData {
  char cmd[32];
  bool obs;
};

String readings; //String to send to other nodes with sensor readings
int lcdColumns = 16;
int lcdRows = 2;
volatile bool human;
volatile double hum, temp;
// int nodeNumber = 2; //Number for this node

LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows);
DHT dht(DHTPIN, DHTTYPE);

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
WiFiUDP udp;

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 30);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings
void receivedCallback( uint32_t from, String &msg );
void readDHT();
void LCD_log(void);
void vRead(void *pvParameters);
void vLCDlog(void *pvParameters);
void vUdpBuff(void *pvParameters);
// void initCommand(myData &cmd, const char *str, bool obs) ;
// void freeCommand(myData &cmd) ;

//-----------------------------------Queue-------------------------------------------------
QueueHandle_t qCMD;

//------------------------------------TASKS-------------------------------------------------
//Create tasks: to send messages and get readings;
Task taskSendMessage(TASK_SECOND , TASK_FOREVER, &sendMessage);
TaskHandle_t Read;
TaskHandle_t LcdLog;

void vRead(void *pvParameters){
  (void)pvParameters;

  pinMode(newConnectLED,OUTPUT);
  dht.begin();
  for (;;){
    readDHT();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vLcdLog(void *pvParameters){
  (void)pvParameters;

  pinMode(obsLED,OUTPUT);   
  for (;;){
    LCD_log();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

//-----------------------------------functions-----------------------------------------
// void freeCommand(myData &data) {
//     free(data.cmd);
// }

// Function to initialize a Command struct
// void initCommand(myData &cmd, const char *str, bool obs) {
//     // Allocate memory for the string
//     cmd.cmd = strdup(str); // Using strdup to dynamically allocate and copy the string
//     if (cmd.cmd == NULL) {
//         Serial.println("Memory allocation failed.");
//         return;
//     }
//     cmd.obs = obs;
// }

String getReadings () {
  JSONVar jsonReadings;
  jsonReadings["node"] = 2;
  jsonReadings["temp"] = temp;
  jsonReadings["hum"] = hum;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void LCD_log(void){
  myData data;
  // uint8_t count = 0;
  if(qCMD != NULL){
    int ret = xQueueReceive(qCMD, &data, (TickType_t)10);
    if(ret == pdPASS){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(data.cmd);
      // for (uint8_t i=0; i<32; i++){
      //   if (i<16){
      //     lcd.setCursor(0,i);
      //     lcd.print(data.cmd[i]);
      //     vTaskDelay(1);
      //   } else {
      //     lcd.setCursor(1,i-16);
      //     lcd.print(data.cmd[i]);
      //     vTaskDelay(1);
      //   }
      //   // count++;
      //   // if (count == 32) break;
      // }

      // switch (data.cmd){
      //   case 1   : lcd.clear(); break;
      //   case 2   : lcd.setCursor(0,0); lcd.print("HUMIDITY: ");  lcd.print(hum); break;
      //   case 3   : lcd.setCursor(0,1); lcd.print("TEMPURATURE:");lcd.print(temp);break; 
      //   case 4   : lcd.clear(); break;
      //   default  : 
      //       for (int i=0; i<15; i++){
      //         if (i>0){
      //           lcd.setCursor(i-1,0);
      //           lcd.print(" ");
      //         }
      //         lcd.setCursor(i,0);
      //         lcd.print("@");
              // vTaskDelay(pdMS_TO_TICKS(300));
      //       }
      //       break; 
      // } 
    } else if(ret == pdFALSE){
      Serial.println("Unable to receive data from the Queue");
    }
    memset(data.cmd, 0, sizeof(data.cmd));
  }
}

void readDHT(){
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  // vTaskDelay(pdMS_TO_TICKS(100));
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg ) {
  // Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  myData taking;
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  taking.obs = myObject["human"];
  // taking.cmd = myObject["cmd"];

  digitalWrite(obsLED, taking.obs ? HIGH : LOW);
  // if(qCMD != NULL && uxQueueSpacesAvailable(qCMD) > 0){
  //   int ret = xQueueSend(qCMD, (void*) &taking, 0);
  //   if(ret == pdTRUE){
  //     // The message was successfully sent.
  //   }else if(ret == errQUEUE_FULL){
  //     // Since we are checking uxQueueSpacesAvailable this should not occur, however if more than one task should
  //     //   write into the same queue it can fill-up between the test and actual send attempt
  //     Serial.println("Unable to send data into the Queue");
  //   } // Queue send check
  // }
}

// void newConnectionCallback(uint32_t nodeId) {
//   Serial.printf("New Connection, nodeId = %u\n", nodeId);
//   //vTaskResume(hled);
// }

void changedConnectionCallback() {
  // Serial.printf("Changed connections\n");
  digitalWrite(newConnectLED, !digitalRead(newConnectLED));
}

// void nodeTimeAdjustedCallback(int32_t offset) {
//   Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
// }

void setup() {
  //---------------UDP--------------------
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  udp.begin(localPort);
  Serial.printf("UDP Client : %s:%i \n", WiFi.localIP().toString().c_str(), localPort);
  //--------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------
  lcd.init();       
  lcd.backlight(); // turn on LCD backlight 
  Serial.begin(115200);
  pinMode(LED_BUILTIN, INPUT);
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
  qCMD = xQueueCreate(32, sizeof(myData));
  xTaskCreate(vLcdLog, "log", 1024*5, NULL, 1, &LcdLog);
  xTaskCreate(vRead, "ReadDHT", 4096, NULL, 1, &Read);
  // xTaskCreate(vUdpBuff, "udp", 1024*6, NULL, 0, &UdpBuff);
  //--------------------------------------------------------------------------------

  //------------------------------MESH-----------------------------------------------
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  // mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  //--------------------------------------------------------------------------------
}

void loop() {
  mesh.update();
}

