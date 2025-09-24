#include <Arduino.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

// Mesh network configuration
#define MESH_PREFIX "Bao"
#define MESH_PASSWORD "12345678"
#define MESH_PORT 5555

// Random messages to send to receivers
char randomMessage[][50] = {"Hello there!", "How are you?", "This is a test message.", "ESP32 mesh networking is fun!", "Have a great day!"};

// Mesh objects

Scheduler userScheduler;
painlessMesh mesh;

// Task to send random messages every 1 second
Task testSendMessage(TASK_SECOND * 1, TASK_FOREVER, []() {
  // Pick a random message
  int index = random(0, sizeof(randomMessage) / sizeof(randomMessage[0]));
  char msg[50] = {};
  memcpy(msg, randomMessage[index], sizeof(msg) - 1);
  Serial.printf("I am Sending: %s\n", msg);
  mesh.sendBroadcast(msg);
});
// Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, []() {
//   String msg = "Hello from Sender Node " + String(mesh.getNodeId()) + " at " + String(millis());
//   Serial.printf("Sending: %s\n", msg.c_str());
//   mesh.sendBroadcast(msg);
// });

// Callback when message is received
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from node %u: %s\n", from, msg.c_str());
}

// Callback when new node connects
void newConnectionCallback(uint32_t nodeId) {
  Serial.println("==========================");
  Serial.printf("Look, I just found a new node: %u\n", nodeId);
  Serial.println("==========================");
}

// Callback when connections change
void changedConnectionCallback() {
  Serial.printf("Connections changed\n");
  Serial.printf("Connected nodes: ");
  auto nodes = mesh.getNodeList();
  for (auto node : nodes) {
    Serial.printf("%u ", node);
  }
  Serial.println();
}

// Callback for time adjustment
void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Time adjusted. Offset: %d\n", offset);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Sender Node...");
  
  // Initialize mesh
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  
  // Set callbacks
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  
  // Add and enable the send message task
  userScheduler.addTask(testSendMessage);
  testSendMessage.enable();
  
  Serial.printf("Sender Node started with ID: %u\n", mesh.getNodeId());
}

void loop() {
  // This keeps the mesh running
  mesh.update();
}