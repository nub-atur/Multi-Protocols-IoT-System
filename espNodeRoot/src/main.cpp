// #define DEBUG_ka

#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <SPI.h>             // include libraries
#include <LoRa.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>

#define   MESH_PREFIX     "RNTMESH"
#define   MESH_PASSWORD   "MESHpassword"
#define   MESH_PORT       5555
#define   LED             25
#define   ss              5
#define   rst             14
#define   dio0            13

// int nodeNumber = 3;

String outgoing;                   // outgoing message
// volatile bool ACKFlag = false;  // response ACK
long lastSendTime = 0;             // last send time
int interval = 2000;               // interval between sends

Preferences prf;
String readings;
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// prototypes
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings
void receivedCallback( uint32_t from, String &msg );
void vLoRa_go(void *pvParameters);
void vHandleACK(void *pvParameters); 
void loraSendMessage(String outgoing);
void onReceive(int packetSize);

//------------------------------------Tasks and Queue-------------------------------------------------------------
SemaphoreHandle_t xSemaphore;           // Handle for the semaphore
Task taskSendMessage(TASK_SECOND, TASK_FOREVER, &sendMessage);
TaskHandle_t LoRa_go;
TaskHandle_t handleACK;
QueueHandle_t commands;

void vHandleACK(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY)){
      String message = prf.getString("ack");
      
      const char* myCharPointer = message.c_str();

      if((strcmp(myCharPointer, "OK") == 0) || (strcmp(myCharPointer, "KO+F") == 0) || (strcmp(myCharPointer, "KO+Q") == 0)){
        loraSendMessage(message);
        #ifdef DEBUG_ka
          Serial.println("Sending: " + message);
        #endif
        LoRa.receive();
      }
    }
  }
}

void vLoRa_go(void * pvParameters){
  (void)pvParameters;

  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 MHz
    #ifdef DEBUG_ka
      Serial.println("LoRa init failed. Check your connections.");
    #endif
    while (true);                       // if failed, do nothing
  }
  
  LoRa.onReceive(onReceive);
  LoRa.receive(); 
  for (;;){
    if (xTaskGetTickCount() - lastSendTime > interval) 
    {
      String message = String(prf.getDouble("temp"), 2);
      message += " ";
      message += String(prf.getDouble("hum"), 2);
      message += " ";
      message += (prf.getBool("human") == true)? "yes" : "no";
      message += " ";
      message += String(lastSendTime/1000);
      loraSendMessage(message);
      lastSendTime = xTaskGetTickCount();            // timestamp the message
      interval = random(2000) + 1000;                // 2-3 seconds

      #ifdef DEBUG_ka
        Serial.println("Sending: " + message);
      #endif

      LoRa.receive();                                // go back into receive mode
    }
                                    
    vTaskDelay(1000);
  } 
}

//-----------------------------------------------------------------------------------------
void loraSendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it                     
}

String getReadings () {
  char receivedMessage[32];
  if (uxQueueMessagesWaiting(commands) > 0) {
    xQueueReceive(commands, &receivedMessage, 10); // Receive message from queue
    digitalWrite(LED, !digitalRead(LED));
  } else strcpy(receivedMessage, "null");

  JSONVar jsonReadings;
  jsonReadings["node"] = 3;
  jsonReadings["cmd"] = receivedMessage;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg ) {
  #ifdef DEBUG_ka
    Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  #endif
  
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  
  if (node == 1)
  {
    boolean human = myObject["human"];
    prf.putBool("human", human);
  } 
  else if (node == 2)
  {
    double temp = myObject["temp"];
    double hum = myObject["hum"];
    String ack = myObject["ack"];
    
    prf.putDouble("temp", round(temp*100)/100);
    prf.putDouble("hum", round(hum*100)/100);
    
    if((strcmp(ack.c_str(), "OK") == 0) || (strcmp(ack.c_str(), "KO+F") == 0) || (strcmp(ack.c_str(), "KO+Q") == 0)){
      xSemaphoreGive(xSemaphore);
      prf.putString("ack", ack);
    }
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

void  onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  String res = "";                      //gain a string from server
  while (LoRa.available()) {
    res += (char)LoRa.read();
  }

  char incoming[32];                    //cast to char[]
  strncpy(incoming, res.c_str(), sizeof(res));

  if (uxQueueSpacesAvailable(commands) > 0) {
    xQueueSend(commands, &incoming, 0); // Send message to queue
  }

  #ifdef DEBUG_ka
    Serial.println("Receiving: " + String(incoming));
    Serial.println();
  #endif

  vTaskDelay(1000);
}

void setup() {
  prf.begin("data");
  pinMode(LED, OUTPUT); 

  xSemaphore = xSemaphoreCreateBinary();          // Create the semaphore
  commands = xQueueCreate(2, sizeof(char[32]));   //Create Queue
  xTaskCreate(vLoRa_go, "LoRa", 1024*5, NULL, 0, &LoRa_go);
  xTaskCreate(vHandleACK, "ACK", 1024*2, NULL, 0, &handleACK);

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  // mesh.onNewConnection(&newConnectionCallback);
  // mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);   //Start task
  taskSendMessage.enable();

  #ifdef DEBUG_ka
    // Check if semaphore was created successfully
    if (xSemaphore == NULL) {
        Serial.println("Semaphore creation failed");
        while (1);
    }
    Serial.begin(115200);
    Serial.println("Done!");
    vTaskDelay(50);
  #endif
}

void loop() {
  mesh.update();
}
