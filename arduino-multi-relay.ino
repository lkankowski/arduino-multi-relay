// Enable debug prints to serial monitor
#define MY_DEBUG

#define MY_GATEWAY_SERIAL
#define MY_INCLUSION_MODE_FEATURE
#define MY_INCLUSION_BUTTON_FEATURE
#define MY_INCLUSION_MODE_DURATION 60
#define MY_INCLUSION_MODE_BUTTON_PIN  3

#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>


int myRelaysPins[] = {2, 4, 5, 6, 7, 8, 9, 10};
int myButtonsPins[] = {A5, A4, A3, A2, A1, A0, 11, 12};
#define RELAY_TRIGGER_LEVEL LOW

const int numberOfRelays = sizeof(myRelaysPins) / sizeof(myRelaysPins[0]);
const int numberOfButtons = sizeof(myButtonsPins) / sizeof(myButtonsPins[0]);

// MySensors - Sending Data
// To send data you have to create a MyMessage container to hold the information.
MyMessage msgs[numberOfRelays];

Bounce myButtonDebouncer[numberOfButtons];


// MySensors - This will execute before MySensors starts up
void before() {
  for (int i = 0; i < numberOfRelays; i++) {
    // Then set relay pins in output mode
    pinMode(myRelaysPins[i], OUTPUT);
    // Set relay to last known state (using eeprom storage)
    uint8_t isTurnedOn = loadState(i); // 1 - true, 0 - false
    uint8_t state = isTurnedOn ? RELAY_TRIGGER_LEVEL : ~RELAY_TRIGGER_LEVEL;
    digitalWrite(myRelaysPins[i], state);
  }
}

// executed AFTER mysensors has been initialised.
void setup() {
  Serial.begin(57600);
  // Setup locally attached sensors
  delay(5000);
  // Setup Relays
  for(int i = 0; i < numberOfRelays; i++) {
    msgs[i] = MyMessage(i, V_LIGHT);
  }
  // Setup buttons
  for(int i = 0; i < numberOfButtons; i++) {
    pinMode(myButtonsPins[i], INPUT_PULLUP);
    // After setting up the button, setup debouncer.
    myButtonDebouncer[i] = Bounce();
    myButtonDebouncer[i].attach(myButtonsPins[i]);
    myButtonDebouncer[i].interval(5);
  }
  //presentation();
}

void loop() {
  for(int i = 0; i < numberOfButtons; i++) {
    if (myButtonDebouncer[i].update()) {
      int buttonState = myButtonDebouncer[i].read();
#ifdef MY_DEBUG
      Serial.print("Button ");
      Serial.print(i);
      Serial.print(" changed to: ");
      Serial.println(buttonState);
#endif
      
      // Button was pushed and now is released
      if (buttonState == LOW) {
        uint8_t isTurnedOn = loadState(i); // 1 - true, 0 - false
        uint8_t newState = isTurnedOn ? ~RELAY_TRIGGER_LEVEL : RELAY_TRIGGER_LEVEL;
        saveState(i, ~isTurnedOn);
        digitalWrite(myRelaysPins[i], newState);
        send(msgs[i].set(~isTurnedOn));
      }
    }
  }
}



// MySensors - Presentation
// Your sensor must first present itself to the controller.
// The presentation is a hint to allow controller prepare for the sensor data that eventually will come.
// void present(uint8_t childSensorId, uint8_t sensorType, const char *description, bool ack);
//   childSensorId - The unique child id you want to choose for the sensor connected to this Arduino. Range 0-254.
//   sensorType - The sensor type you want to create.
//   description An optional textual description of the attached sensor.
//   ack - Set this to true if you want destination node to send ack back to this node. Default is not to request any ack.
void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Multi Relay", "1.0");
	
	// Register every relay as separate sensor
  for (int i = 0; i < numberOfRelays; i++) {
    // Register all sensors to gw (they will be created as child devices)
    present(i, S_LIGHT);
  }
}


// MySensors - Handling incoming messages
// Nodes that expects incoming data, such as an actuator or repeating nodes,
// must implement the receive() - function to handle the incoming messages.
// Do not sleep a node where you expect incoming data or you will lose messages.
void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_LIGHT) {
    uint8_t isTurnedOn = message.getBool(); // 1 - true, 0 - false
    uint8_t newState = isTurnedOn ? RELAY_TRIGGER_LEVEL : ~RELAY_TRIGGER_LEVEL;
    // Change relay state
    digitalWrite(myRelaysPins[message.sensor], newState);
    // Store state in eeprom if changed
    if (loadState(message.sensor) != isTurnedOn) {
      saveState(message.sensor, message.getBool());
    }
    // Write some debug info
    Serial.print("Incoming change for sensor: " + message.sensor);
    Serial.println(", New status: " + message.getBool());
  }
}

