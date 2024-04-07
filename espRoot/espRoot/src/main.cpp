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
#define   dio0            27

//--------------------Queueu-----------------------------------------
const int QueueElementSize = 20;
QueueHandle_t QueueHandle;

typedef struct{
  // String data;
  bool human;
  double temp;
  double hum;
} message_t;

// String data;

int nodeNumber = 3;

volatile long lastSendTime = 0;        // last send time

Preferences prf;
String readings;
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// volatile bool human;
// volatile double temp;
// volatile double hum;
volatile unsigned char cmd_buffer;

// prototypes
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings
void receivedCallback( uint32_t from, String &msg );
void vLoRa_go(void * pvParameters);
// void vLora_back(void * pvParameters);
void loraSendMessage(String outgoing);
void onReceive(int packetSize);
// void LoRa_rxMode();
// void LoRa_txMode();
// void onTxDone();

//------------------------------------TASKS-------------------------------------------------------------
Task taskSendMessage(TASK_SECOND, TASK_FOREVER, &sendMessage);

// TaskHandle_t LoRa_back;
// void vLora_back(void * pvParameters){
//   (void)pvParameters;
//   for(;;){
//     LoRa_rxMode();
//     // LoRa.enableInvertIQ();                // active invert I and Q signals
//     // LoRa.receive();                       // set receive mode
//     onReceive(LoRa.parsePacket());
//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

TaskHandle_t LoRa_go;
void vLoRa_go(void * pvParameters){
  (void)pvParameters;
  message_t tx_data;
  // LoRa.onReceive(onReceive);
  // LoRa.onTxDone(onTxDone);
  // LoRa.onReceive(onReceive);
  // LoRa.onTxDone(onTxDone);
  // LoRa_rxMode();
  for (;;){
    // LoRa_txMode();
      // LoRa.idle();                          // set standby mode
      // LoRa.disableInvertIQ();               // normal mode
    if (xTaskGetTickCount() - lastSendTime > pdMS_TO_TICKS(2000)) {
      String message = "";   // send a message

      // message += String(prf.getDouble("temp"),6);
      // message += String(prf.getDouble("hum"),6);
      // message +=  (prf.getBool("ob") == true) ? "Detected Obstacle" : "No obstacle"; 

      if(QueueHandle != NULL){
        int ret = xQueueReceive(QueueHandle, &tx_data, portMAX_DELAY);
        if(ret == pdPASS){
          if (!isnan(tx_data.temp) && !isnan(tx_data.hum)){
            message += String(tx_data.temp,6);
            message += String(tx_data.hum,6);
            message +=  (prf.getBool("ob") == true) ? "Detected Obstacle" : "No obstacle";
          } else {
            message += String(prf.getDouble("temp"),6);
            message += String(prf.getDouble("hum"),6);
            message +=  (prf.getBool("ob") == true) ? "Detected Obstacle" : "No obstacle"; 
          }
          // else message = "Mesh not working now!";
          // message += String(tx_data.temp,6);
          // message += String(tx_data.hum,6);
          // message += (tx_data.human == true)? "Detected Obstacle" : "No obstacle"; 
          // message = tx_data;
        } else if(ret == pdFALSE){
          Serial.println("Unable to receive data from the Queue");
        }
      }
      loraSendMessage(message);
      Serial.println("Sending " + message);
      // vTaskDelay(pdMS_TO_TICKS(1000));
      lastSendTime = xTaskGetTickCount();            // timestamp the message    
    } 
    onReceive(LoRa.parsePacket());
    
  } 
}

void loraSendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                 // finish packet and send it       
  // LoRa_rxMode();       
}

// void onTxDone() {
//   Serial.println("TxDone");
//   LoRa_rxMode();
// }

// void LoRa_rxMode(){
//   LoRa.enableInvertIQ();                // active invert I and Q signals
//   LoRa.receive();                       // set receive mode
// }

// void LoRa_txMode(){
//   LoRa.idle();                          // set standby mode
//   LoRa.disableInvertIQ();               // normal mode
// }

void onReceive(int packetSize) {
  digitalWrite(LED, !digitalRead(LED));
  if (packetSize == 0) return;   
   // if the packer size is not 0, then execute this if condition
  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  // int i = 0;
  // while (incoming[i] != '\0') {
  //   if (isdigit(incoming[i])) {
  //     cmd_buffer = incoming[i];
  //     break;
  //   }
  //   i++;
  // }
  // cmd_buffer = incoming.charAt(0);
  
  // LoRa_txMode(); 
  
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

//--------------------------Mesh received Callback----------------------------
String getReadings () {
  JSONVar jsonReadings;
  jsonReadings["node"] = nodeNumber;
  jsonReadings["cmd"] = cmd_buffer;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg ) { 
  // Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  // Serial.println();
  message_t message;
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  message.human = myObject["human"];
  message.temp = myObject["temp"];
  message.hum = myObject["hum"];

  // String data = "";
  
  if (node == 1) {
    prf.putBool("ob",message.human);  //save to EEPROM
  } else if (node == 2){
    prf.putDouble("temp",message.temp);
    prf.putDouble("hum",message.hum);
  } 

  if(QueueHandle != NULL && uxQueueSpacesAvailable(QueueHandle) > 0){
    int ret = xQueueSend(QueueHandle, (void*) &message, 0);
    if(ret == pdTRUE){
      // The message was successfully sent.
    }else if(ret == errQUEUE_FULL){
      // Since we are checking uxQueueSpacesAvailable this should not occur, however if more than one task should
      //   write into the same queue it can fill-up between the test and actual send attempt
      Serial.println("The `Mesh ReceivedCallback` was unable to send data into the Queue");
    } // Queue send check
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

void setup() {
  Serial.begin(115200);
  while (!Serial);
  pinMode(LED, OUTPUT);
  prf.begin("data");

  LoRa.setPins(ss, rst, dio0);          // init LoRa SPI
  if (!LoRa.begin(433E6)) {             // initialize ratio
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  // LoRa.onReceive(onReceive);
  // LoRa.onTxDone(onTxDone);
  // LoRa_rxMode();

  QueueHandle = xQueueCreate(QueueElementSize, sizeof(message_t));                //-------------
  xTaskCreatePinnedToCore(vLoRa_go, "LoRa_go", 4000, NULL, 0, &LoRa_go, 0);       //--init tasks  
  // xTaskCreatePinnedToCore(vLora_back, "LoRa_back", 1000, NULL, 2, &LoRa_back, 0); //------------

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  // mesh.onNewConnection(&newConnectionCallback);
  // mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}