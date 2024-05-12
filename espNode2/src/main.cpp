// #define DEBUG

#include "DHT.h"
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

#define DHTPIN         25 
#define meshLED        12
#define loraLED        14
#define DHTTYPE        DHT22
#define MESH_PREFIX    "RNTMESH" //name for your MESH
#define MESH_PASSWORD  "MESHpassword" //password for your MESH
#define MESH_PORT      5555 //default port

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows);
DHT dht(DHTPIN, DHTTYPE);

int nodeNumber = 2;                   //Number for this node
String readings;                      //String to send to other nodes with sensor readings
volatile boolean comingFlag = false;  //Checking if message is received from LoRa

Scheduler userScheduler;              // to control your personal task
painlessMesh mesh;

// User stub
void vTaskLCD(void * pvParameters);
void sendMessage() ;                 // Prototype so PlatformIO doesn't complain
String getReadings();                // Prototype for sending sensor readings
void receivedCallback( uint32_t from, String &msg );

//Create tasks: to send messages and get readings;
QueueHandle_t myQueue;
TaskHandle_t taskLCD;
Task taskSendMessage(TASK_SECOND, TASK_FOREVER, &sendMessage);

void vTaskLCD(void * pvParameters){
  for (;;){
    digitalWrite(loraLED, (comingFlag == true)? HIGH : LOW);
    if (comingFlag == true){
      lcd.clear();
      if (uxQueueMessagesWaiting(myQueue) > 0) {
        char receivedMessage[32];
        xQueueReceive(myQueue, &receivedMessage, 10); // Receive message from queue
        for (uint8_t i=0; i<32; i++){
          if (!isalpha(receivedMessage[i]) && (receivedMessage[i] != 32)) break;
          if (i<16){
            lcd.setCursor(i, 0);
          } else {
            lcd.setCursor(i-16, 1);
          }
          lcd.print(receivedMessage[i]);
        }
      }
      comingFlag = false;
    }
    vTaskDelay(2000);
  }
}

String getReadings () {
  JSONVar jsonReadings;
  jsonReadings["node"] = nodeNumber;
  jsonReadings["temp"] = roundf(dht.readTemperature()*100)/100;
  jsonReadings["hum"] = roundf(dht.readHumidity()*100)/100;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg ) {
  #ifdef DEBUG
    Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  #endif

  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];

  switch (node){
    case 1:{ 
      boolean human = myObject["human"]; 
      digitalWrite(meshLED, (human == true)? HIGH : LOW); 
      break;
    }    
    case 3:{ 
      char cmd[32];
      strcpy(cmd, myObject["cmd"]);
      if (strcmp(cmd, "null") != 0){
        comingFlag = true;
        if (uxQueueSpacesAvailable(myQueue) > 0) {
          xQueueSend(myQueue, &cmd, 0); // Send message to queue
        }
      }
      break;
    }
    default: break;
  }
}

// void newConnectionCallback(uint32_t nodeId) {
//   Serial.printf("New Connection, nodeId = %u\n", nodeId);
// }

// void changedConnectionCallback() {
//   Serial.printf("Changed connections\n");
// }

// void nodeTimeAdjustedCallback(int32_t offset) {
//   Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
// }

void setup() {
  pinMode(meshLED,OUTPUT);
  pinMode(loraLED,OUTPUT);
  
  // turn on LCD backlight     
  lcd.init();                 
  lcd.backlight();

  dht.begin();

  myQueue = xQueueCreate(2, sizeof(char[32]));
  xTaskCreatePinnedToCore(vTaskLCD, "lcd", 1024*5, NULL, 1, &taskLCD, 0);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  // mesh.onNewConnection(&newConnectionCallback);
  // mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println(F("Done!"));
  #endif
}

void loop() {
  mesh.update();
}
