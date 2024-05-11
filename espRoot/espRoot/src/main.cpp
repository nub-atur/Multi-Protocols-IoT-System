#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <SPI.h>             // include libraries
#include <LoRa.h>
#include <Preferences.h>
#include <ctype.h>

#define   MESH_PREFIX     "RNTMESH"
#define   MESH_PASSWORD   "MESHpassword"
#define   MESH_PORT       5555
#define   LED             33
#define   ss              5
#define   rst             14
#define   dio0            26 // GPIO26 IRQ(Interrupt Request)

//--------------------Queueu-----------------------------------------
// const int QueueElementSize = 20;
QueueHandle_t QueueHandle;

typedef struct{
  bool human;
  double temp;
  double hum;
} message_t;

int nodeNumber = 3;
// byte msgCount = 0;            // count of outgoing messages
// byte localAddress = 0xBB;     // address of this device
// byte destination = 0xFF;      // destination to send to
volatile long lastSendTime = 0;        // last send time

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

//------------------------------------TASKS-------------------------------------------------------------
Task taskSendMessage(TASK_SECOND, TASK_FOREVER, &sendMessage);

TaskHandle_t LoRa_go;
void vLoRa_go(void * pvParameters){
  (void)pvParameters;
  // uint8_t counter = 0; 
  for (;;){
    if (xTaskGetTickCount() - lastSendTime > pdMS_TO_TICKS(4000)) {
      String message = "";   // send a message
      message += String(prf.getDouble("temp"),2);
      message += " ";
      message += String(prf.getDouble("hum"),2);
      message += " ";
      message +=  (prf.getBool("ob") == true) ? "yes" : "no"; 
      message += " ";
      message += pdTICKS_TO_MS(xTaskGetTickCount())/1000;
      // counter++;
      loraSendMessage(message); //test "message" data
      // Serial.println("Sending " + message);
      // Serial.printf("\t%d\n", counter);
      // if (counter == 5) counter = 0;

      lastSendTime = xTaskGetTickCount();            // timestamp the message    
    } 
    onReceive(LoRa.parsePacket());
    vTaskDelay(200 / portTICK_PERIOD_MS);
  } 
}

void loraSendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                 // finish packet and send it       
  // msgCount++;    
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  char incoming[32] = "";

  while (LoRa.available()) {
    char receivedChar = (char)LoRa.read();
    strncat(incoming, &receivedChar, 1);
  }
  
  if(QueueHandle != NULL && uxQueueSpacesAvailable(QueueHandle) > 0){
    int ret = xQueueSend(QueueHandle, (void*) &incoming, 0);
    if(ret == pdTRUE){
      // The message was successfully sent.
    }else if(ret == errQUEUE_FULL){
      // Since we are checking uxQueueSpacesAvailable this should not occur, however if more than one task should
      //   write into the same queue it can fill-up between the test and actual send attempt
      Serial.println("The `Mesh ReceivedCallback` was unable to send data into the Queue");
    } // Queue send check
  }

  Serial.println("Message: " + String(incoming));
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

//--------------------------Mesh received Callback----------------------------
String getReadings () {
  char cmd_buffer[32];
  if(QueueHandle != NULL){
    if(xQueueReceive(QueueHandle, &cmd_buffer, 50) == pdPASS){
      Serial.println("boardcast command now...");
    } else {
      strcpy(cmd_buffer, "null");
    //Serial.println("The `Mesh getReadings` was unable to receive data from the Queue");
    }
  } 

  JSONVar jsonReadings;
  jsonReadings["node"] = nodeNumber;
  jsonReadings["cmd"] = String(cmd_buffer);
  
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg ) { 
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  // Serial.println();
  message_t message;
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  message.human = myObject["human"];
  message.temp = myObject["temp"];
  message.hum = myObject["hum"];
  
  if (node == 1) {
    prf.putBool("ob",message.human);  //save to EEPROM
  } else if (node == 2){
    prf.putDouble("temp",message.temp);
    prf.putDouble("hum",message.hum);
  } 
} 

void newConnectionCallback(uint32_t nodeId) {
  // Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  // Serial.printf("Changed connections\n");
  digitalWrite(LED, !digitalRead(LED));
}

void nodeTimeAdjustedCallback(int32_t offset) {
  // Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  // while (!Serial);

  pinMode(LED, OUTPUT);

  prf.begin("data");

  LoRa.setPins(ss, rst, dio0);          // init LoRa SPI
  if (!LoRa.begin(433E6)) {             // initialize ratio
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  LoRa.setSpreadingFactor(7);
  LoRa.setSyncWord(0x12);
  LoRa.setSignalBandwidth(125000);
  LoRa.setCodingRate4(5);
  LoRa.setTxPower(17);
  
  QueueHandle = xQueueCreate(32, sizeof(char[32]));                                //-------------
  xTaskCreate(vLoRa_go, "LoRa_go", 1024*3, NULL, 1, &LoRa_go);                    //--init tasks  
  // xTaskCreate(vLora_back, "LoRa_back", 1024*2, NULL, 1, &LoRa_back);           //------------

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  // mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}