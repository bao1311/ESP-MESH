#include <Arduino.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>


// Mesh network configuration (must match sender)
#define MESH_PREFIX "Imposter"
#define MESH_PASSWORD "12345678"
#define MESH_PORT 5555

// Random answer back to sender
char answer[][50] = {"I am Imposter", "All your base are belong to us", "The cake is a lie", "Hello from the other side", "I see dead pixels"};
// Mesh objects
Scheduler userScheduler;
painlessMesh mesh;

// Task to send acknowledgment messages every 2 seconds (Not really make sense since we send ack only when we receive a message, but just for demo)

Task taskSendAck(TASK_SECOND * 2, TASK_FOREVER, []() {
  char ackMsg[100] = "Hey, I got your message! I am Imposter";
  strcat(ackMsg, " - Random fact: ");
  strcat(ackMsg, answer[random(0, sizeof(answer) / sizeof(answer[0]))]);
  Serial.printf("I am sending this back to the sender: %s\n", ackMsg);
  mesh.sendBroadcast(ackMsg);
});

// Callback when message is received
void receivedCb(uint32_t from, char *msg) {
  Serial.printf("Hey I received a message from %u: %s\n", from, msg);
}

void receivedCallbackLegit(uint32_t from, char* msg)
{
  // Debug print
  Serial.printf("Hey I received a message from %u: %s\n", from, msg);
  // Send acknowledgment back to sender 
  char ackMsg[100] = "Hey, I got your message! I am Imposter";
  // Generate data
  strcat(ackMsg, " - Random fact: ");
  strcat(ackMsg, answer[random(0, sizeof(answer) / sizeof(answer[0]))]);
  Serial.printf("I am sending this back to the sender: %s\n", ackMsg);
  // Send
  mesh.sendBroadcast(ackMsg);
}

// Callback when new node connects
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("Hey, we got a new friend: %u\n", nodeId);
}

// Callback when node disconnects
void droppedConnectionCallback(uint32_t nodeId) {
  Serial.printf("Hey, our friend %u disconnected\n", nodeId);
}

// Callback when connections change
void changedConnectionCallback() {
  Serial.printf("Connections changed\n");
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
  
  
  // Set debug message types before init()
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
  
  // Initialize mesh
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  
  // Set callbacks
  mesh.onReceive(&receivedCallbackLegit);
  mesh.onNewConnection(&newConnectionCallback);
  // mesh.onDroppedConnection(&droppedConnectionCallback);
  // mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  
  // Add and enable the acknowledgment task
  userScheduler.addTask(taskSendAck);
  taskSendAck.enable();
  
  Serial.printf("Receiver Node started with ID: %u\n", mesh.getNodeId());
  Serial.println("Ready to receive messages...");
}

void loop() {
  // This keeps the mesh running
  mesh.update();
}