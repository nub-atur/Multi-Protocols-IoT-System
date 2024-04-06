#include "DHT.h"
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <LiquidCrystal_I2C.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define DHTPIN 25 
#define obsLED 12
#define newConnectLED 14
#define DHTTYPE DHT22
#define MESH_PREFIX    "RNTMESH" //name for your MESH
#define MESH_PASSWORD  "MESHpassword" //password for your MESH
#define MESH_PORT      5555 //default port

//--------------------------------------------------
//            Task testing
// TaskHandle_t hled;
// void vLEDFlashTask(void *pvParameters);

// void vLEDFlashTask(void *pvParameters) // This is a task.
// {
//   (void)pvParameters;

//   // initialize digital LED_BUILTIN on pin 2 as an output.
//   Serial.print(F("LEDTask at core:"));
//   Serial.println(xPortGetCoreID());
//   pinMode(LED_BUILTIN, OUTPUT);
//   for (;;) // A Task shall never return or exit.
//   {
//     digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
//     vTaskDelay(2000);
//     digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
//     vTaskDelay(2000);
    
//   }
// }
//--------------------------------------------------

struct myData {
  int cmd;
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

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings
void receivedCallback( uint32_t from, String &msg );
void readDHT();
void LCD_log(void);
void vRead(void *pvParameters);
void vLCDlog(void *pvParameters);
//Queue//
QueueHandle_t qCMD;

//------------------------------------TASKS-------------------------------------------------
//Create tasks: to send messages and get readings;
Task taskSendMessage(TASK_SECOND /2 , TASK_FOREVER, &sendMessage);
TaskHandle_t Read;
TaskHandle_t LCDlog;

// TaskHandle_t hled; //for testing RTOS
// void vLEDFlashTask(void *pvParameters);
// void vLEDFlashTask(void *pvParameters) // This is a task.
// {
//   (void)pvParameters;

//   // initialize digital LED_BUILTIN on pin 2 as an output.
//   Serial.print(F("LEDTask at core:"));
//   Serial.println(xPortGetCoreID());
//   pinMode(LED_BUILTIN, OUTPUT);
//   for (;;) // A Task shall never return or exit.
//   {
//     digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
//     vTaskDelay(2000);
//     digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
//     vTaskDelay(2000);
//   }
// }

void vRead(void *pvParameters){
  (void)pvParameters;

  pinMode(newConnectLED,OUTPUT);
  dht.begin();
  for (;;){
    readDHT();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void vLCDlog(void *pvParameters){
  (void)pvParameters;

  pinMode(obsLED,OUTPUT);   
  for (;;){
    LCD_log();
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

//-----------------------------------functions-----------------------------------------
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

  if(qCMD != NULL){
    int ret = xQueueReceive(qCMD, &data, portMAX_DELAY);
    if(ret == pdPASS){
      digitalWrite(obsLED, data.obs ? HIGH : LOW);
      switch (data.cmd){
        case 1   : lcd.clear(); break;
        case 2   : lcd.setCursor(0,0); lcd.print("HUMIDITY: ");  lcd.print(hum); break;
        case 3   : lcd.setCursor(0,1); lcd.print("TEMPURATURE:");lcd.print(temp);break; 
        case 4   : lcd.clear(); break;
        default  : 
            for (int i=0; i<15; i++){
              if (i>0){
                lcd.setCursor(i-1,0);
                lcd.print(" ");
              }
              lcd.setCursor(i,0);
              lcd.print("@");
              vTaskDelay(pdMS_TO_TICKS(200));
            }
            break; 
      } 
    } else if(ret == pdFALSE){
      Serial.println("Unable to receive data from the Queue");
    }
  }
}

void readDHT(){
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  vTaskDelay(pdMS_TO_TICKS(100));
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  myData taking;
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];
  taking.obs = myObject["human"];
  taking.cmd = myObject["cmd"];

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
  lcd.init();       
  lcd.backlight(); // turn on LCD backlight 
  Serial.begin(115200);
  pinMode(LED_BUILTIN, INPUT);

  //---------------------------task test------------
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
  // //--------------------------------------------------
  qCMD = xQueueCreate(5, sizeof(myData));
  xTaskCreate(vLCDlog, "log", 4096, NULL, 0, &LCDlog);
  xTaskCreate(vRead, "ReadDHT", 4096, NULL, 1, &Read);
  
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
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

