#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <SPI.h>             // include libraries
#include <LoRa.h>
#include <Preferences.h>

#define   MESH_PREFIX     "RNTMESH"
#define   MESH_PASSWORD   "MESHpassword"
#define   MESH_PORT       5555
#define   LED             33
#define   ss 5
#define   rst 14
#define   dio0 27

int nodeNumber = 3;

// //String outgoing;              // outgoing message
// String LoRaData;

//byte msgCount = 0;            // count of outgoing messages
// byte localAddress = 0xBB;     // address of this device
// byte destination = 0xFF;      // destination to send to
volatile long lastSendTime = 0;        // last send time
//int interval = 1000;          // interval between sends

Preferences prf;
String readings;
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

volatile bool human;
volatile double temp;
volatile double hum;
volatile char cmd_buffer;

// prototypes
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings
void receivedCallback( uint32_t from, String &msg );
void vLoRa_go(void * pvParameters);
//void vSaveEEP(void * pvParameters);
void loraSendMessage(String outgoing);
void onReceive(int packetSize);

//------------------------------------TASKS-------------------------------------------------------------
Task taskSendMessage(TASK_SECOND/2, TASK_FOREVER, &sendMessage);

TaskHandle_t LoRa_go;
void vLoRa_go(void * pvParameters){
  (void)pvParameters;

  for (;;){
    if (xTaskGetTickCount() - lastSendTime > pdMS_TO_TICKS(2000)) {
      String message = "";   // send a message

      message += String(prf.getDouble("temp"),6);
      message += String(prf.getDouble("hum"),6);
      if (prf.getBool("ob") == true) 
        message += "Detected Obstacle"; 
      else message += "No obstacle"; 

      loraSendMessage(message);

      Serial.println("Sending " + message);
      lastSendTime = xTaskGetTickCount();            // timestamp the message
    } 
    onReceive(LoRa.parsePacket());
  } 
}

void loraSendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it              
}

String getReadings () {
  JSONVar jsonReadings;
  jsonReadings["node"] = nodeNumber;
  jsonReadings["cmd"] = cmd_buffer;
  readings = JSON.stringify(jsonReadings);
  digitalWrite(LED, !digitalRead(LED));
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  Serial.println();
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  human = myObject["human"];
  temp = myObject["temp"];
  hum = myObject["hum"];
  
  if (node == 1) prf.putBool("ob",human);
  else {
    prf.putDouble("temp",temp);
    prf.putDouble("hum",hum);
  } 
}

void newConnectionCallback(uint32_t nodeId) {
  // Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  // Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  // Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;   
   // if the packer size is not 0, then execute this if condition
  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  
  cmd_buffer = incoming.charAt(0);

  Serial.print("Message: ");
  Serial.print(cmd_buffer);
  Serial.print("      RSSI: " + String(LoRa.packetRssi()));
  Serial.println("      Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  pinMode(LED, OUTPUT);
  prf.begin("data");

  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {             // initialize ratio
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  xTaskCreatePinnedToCore(vLoRa_go, "LoRa", 8192, NULL, 0, &LoRa_go, 0);
  //xTaskCreate(vSaveEEP, "Save", 4096, NULL, 1, &SaveEEP);

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}