#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <Preferences.h>

// MESH Details
#define   MESH_PREFIX     "RNTMESH" //name for your MESH
#define   MESH_PASSWORD   "MESHpassword" //password for your MESH
#define   MESH_PORT       5555 //default port

int nodeNumber = 1;//Number for this node
volatile bool human;
int lastState = HIGH;  // the previous state from the input pin
int currentState;      // the current reading from the input pin
String readings; //String to send to other nodes with sensor readings

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

Preferences prf;    //save data

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings

//Create tasks: to send messages and get readings;
Task taskSendMessage(TASK_SECOND/2, TASK_FOREVER, &sendMessage);

String getReadings () {
  currentState = digitalRead(4);
  if (lastState == HIGH && currentState == LOW) human = true;
  else if (lastState == LOW && currentState == HIGH) human = false;
  
  JSONVar jsonReadings;
  jsonReadings["node"] = nodeNumber;
  jsonReadings["human"] = human;
  readings = JSON.stringify(jsonReadings);
  
  lastState = currentState;
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  // Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  double temp = myObject["temp"];
  double hum = myObject["hum"];
  // Serial.print("Node: ");
  // Serial.println(node);
  // Serial.print("Temperature: ");
  // Serial.print(temp);
  // Serial.println(" C");
  // Serial.print("Humidity: ");
  // Serial.print(hum);
  // Serial.println(" %");
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
  pinMode(4, INPUT);
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
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
  // it will run the user scheduler as well
  mesh.update();
}