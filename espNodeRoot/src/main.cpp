// #define DEBUG

#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <SPI.h>             // include libraries
#include <LoRa.h>
#include <Preferences.h>

#define   MESH_PREFIX     "RNTMESH"
#define   MESH_PASSWORD   "MESHpassword"
#define   MESH_PORT       5555
#define   LED             33
#define   ss              5
#define   rst             14
#define   dio0            26

int nodeNumber = 3;

String outgoing;              // outgoing message
// boolean commandFlag = false;
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

Preferences prf;
String readings;
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// prototypes
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings
void receivedCallback( uint32_t from, String &msg );
void vLoRa_go(void * pvParameters);
void loraSendMessage(String outgoing);
void onReceive(int packetSize);

//------------------------------------Tasks and Queue-------------------------------------------------------------
Task taskSendMessage(TASK_SECOND, TASK_FOREVER, &sendMessage);
TaskHandle_t LoRa_go;
QueueHandle_t commands;

void vLoRa_go(void * pvParameters){
  (void)pvParameters;

  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 MHz
    #ifdef DEBUG
      Serial.println("LoRa init failed. Check your connections.");
    #endif

    while (true);                       // if failed, do nothing
  }
  
  LoRa.onReceive(onReceive);
  LoRa.receive();
  for (;;){
    if (xTaskGetTickCount() - lastSendTime > interval) {
      String message;  
      message = String(prf.getDouble("temp"));
      message += " ";
      message += String(prf.getDouble("hum"));
      message += " ";
      message += (prf.getBool("human") == true)? "yes" : "no";
      message += " ";
      message += String(lastSendTime/1000);
      loraSendMessage(message);
      // Serial.println("Sending " + message);
      lastSendTime = xTaskGetTickCount();            // timestamp the message
      interval = random(2000) + 1000;    // 2-3 seconds
      LoRa.receive();                     // go back into receive mode
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
  jsonReadings["node"] = nodeNumber;
  jsonReadings["cmd"] = receivedMessage;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  // if (commandFlag == true){
    String msg = getReadings();
    mesh.sendBroadcast(msg);
    // commandFlag = false;
  // } else return;
}

void receivedCallback( uint32_t from, String &msg ) {
  // if (commandFlag == true) return;
  #ifdef DEBUG
    Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  #endif
  
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  
  if (node == 1){
    boolean human = myObject["human"];
    prf.putBool("human", human);
  } else if (node == 2){
    double temp = myObject["temp"];
    double hum = myObject["hum"];
    prf.putDouble("temp", temp);
    prf.putDouble("hum", hum);
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

void onReceive(int packetSize) {
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

  #ifdef DEBUG
    Serial.println("Message: " + String(incoming));
    Serial.println();
  // commandFlag == true;
  #endif

  vTaskDelay(1000);
}

void setup() {
  prf.begin("data");
  pinMode(LED, OUTPUT);

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  // mesh.onNewConnection(&newConnectionCallback);
  // mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  xTaskCreatePinnedToCore(vLoRa_go, "LoRa", 1024*5, NULL, 1, &LoRa_go, 1);
  commands = xQueueCreate(2, sizeof(char[32]));
  taskSendMessage.enable();

  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Done!");
    vTaskDelay(50);
  #endif
}

void loop() {
  mesh.update();
}
