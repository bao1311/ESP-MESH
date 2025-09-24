#include <Arduino.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

// Ensure LED_BUILTIN is defined for platforms that don't provide it
#ifndef LED_BUILTIN
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#define LED_BUILTIN 2
#else
#define LED_BUILTIN 13
#endif
#endif

// Mesh network configuration (must match sender)
#define MESH_PREFIX "Bao"
#define MESH_PASSWORD "12345678"
#define MESH_PORT 5555

// Mesh objects
Scheduler userScheduler;
painlessMesh mesh;

// Task to send acknowledgment messages every 10 seconds
Task taskSendAck(TASK_SECOND * 10, TASK_FOREVER, []() {
  String ackMsg = "ACK from Receiver Node " + String(mesh.getNodeId()) + " at " + String(millis());
  Serial.printf("Sending ACK: %s\n", ackMsg.c_str());
  mesh.sendBroadcast(ackMsg);
});

// Callback when message is received
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("📨 RECEIVED from node %u: %s\n", from, msg.c_str());
  
  // Blink built-in LED to show message received
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Count received messages
  static int messageCount = 0;
  messageCount++;
  Serial.printf("Total messages received: %d\n", messageCount);
}

// Callback when new node connects
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("🔗 New node connected: %u\n", nodeId);
}

// Callback when node disconnects
void droppedConnectionCallback(uint32_t nodeId) {
  Serial.printf("❌ Node %u disconnected\n", nodeId);
}

// Callback when connections change
void changedConnectionCallback() {
  Serial.printf("🔄 Connections changed\n");
  Serial.printf("Connected nodes: ");
  auto nodes = mesh.getNodeList();
  Serial.printf("(%d total) ", nodes.size());
  for (auto node : nodes) {
    Serial.printf("%u ", node);
  }
  Serial.println();
}

// Callback for time adjustment
void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("⏰ Time adjusted. Offset: %d\n", offset);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting PainlessMesh Receiver Node...");
  
  // Initialize built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Set debug message types before init()
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
  
  // Initialize mesh
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  
  // Set callbacks
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onDroppedConnection(&droppedConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  
  // Add and enable the acknowledgment task
  userScheduler.addTask(taskSendAck);
  taskSendAck.enable();
  
  Serial.printf("🎯 Receiver Node started with ID: %u\n", mesh.getNodeId());
  Serial.println("Ready to receive messages...");
  Serial.println("LED will blink when messages are received");
}

void loop() {
  // This keeps the mesh running
  mesh.update();
}