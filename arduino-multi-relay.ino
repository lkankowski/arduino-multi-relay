// Enable debug prints to serial monitor
#define MY_DEBUG

#define MY_GATEWAY_SERIAL
#define MY_INCLUSION_MODE_FEATURE
#define MY_INCLUSION_BUTTON_FEATURE
#define MY_INCLUSION_MODE_DURATION 60
#define MY_INCLUSION_MODE_BUTTON_PIN  3

#include <MySensors.h>
#include <Bounce2.h>

enum ButtonType {
  MONO_STABLE = 0,
  BI_STABLE = 1
};

typedef struct {
  int relay;
  int button;
  uint8_t relayTrigger;
  ButtonType buttonType;
} RelayButton;

// CONFIGURE ONLY THIS ARRAY!
// Row params: relay pin, button pin, relay trigger [HIGH|LOW], button type [MONO_STABLE|BI_STABLE]
RelayButton myRelayButtons[] = {
    {12, A0, LOW, MONO_STABLE},
    {11, A1, LOW, MONO_STABLE},
    {10, A2, LOW, MONO_STABLE},
    {9, A3, LOW, MONO_STABLE},
    {8, A4, LOW, MONO_STABLE},
    {7, A5, LOW, MONO_STABLE},
    {6, A6, LOW, MONO_STABLE},
    {5, A7, LOW, MONO_STABLE},
    {4, A8, LOW, MONO_STABLE},
    {3, A9, LOW, MONO_STABLE},
    {2, A10, LOW, MONO_STABLE},
    {14, A11, LOW, MONO_STABLE},
    {15, A12, LOW, MONO_STABLE},
    {16, A13, LOW, MONO_STABLE},
    {17, A14, LOW, MONO_STABLE},
    {18, A15, LOW, MONO_STABLE},
    {19, 53, LOW, MONO_STABLE},
    {20, 52, LOW, MONO_STABLE},
    {21, 51, LOW, MONO_STABLE},
    {22, 50, LOW, MONO_STABLE},
    {23, 49, LOW, MONO_STABLE},
    {24, 48, LOW, MONO_STABLE},
    {25, 47, LOW, MONO_STABLE},
    {26, 46, LOW, MONO_STABLE},
    {27, 45, LOW, MONO_STABLE},
    {28, 44, LOW, MONO_STABLE},
    {29, 43, LOW, MONO_STABLE},
    {30, 42, LOW, MONO_STABLE},
    {31, 41, LOW, MONO_STABLE},
    {32, 40, LOW, MONO_STABLE},
    {33, 39, LOW, MONO_STABLE},
    {34, 38, LOW, MONO_STABLE}
};

const int numberOfRelayButtons = sizeof(myRelayButtons) / sizeof(RelayButton);

typedef struct {
  int firstButton;
  int nextButton;
} RelayMultiButtons;

RelayMultiButtons relayMultiButtons[numberOfRelayButtons];

// MySensors - Sending Data
// To send data you have to create a MyMessage container to hold the information.
MyMessage msgs[numberOfRelayButtons];

Bounce myButtonDebouncer[numberOfRelayButtons];


// MySensors - This will execute before MySensors starts up
void before() {
  // initialize multiple buttons list structure
  for (int i = 0; i < numberOfRelayButtons; i++) {
    relayMultiButtons[i].firstButton = -1;
    relayMultiButtons[i].nextButton = -1;
  }
  // find multiple buttons for the same relay (uni-directional list)
  for (int i = 0; i < numberOfRelayButtons-1; i++) {
    if (relayMultiButtons[i].firstButton == -1) {
      int prevRelayButton = i;
      for (int j = i+1; j < numberOfRelayButtons; j++) {
        if (myRelayButtons[i].relay == myRelayButtons[j].relay) {
          relayMultiButtons[prevRelayButton].firstButton = i;
          relayMultiButtons[prevRelayButton].nextButton = j;
          relayMultiButtons[j].firstButton = i;
          prevRelayButton = j;
        }
      }
    }
  }
  
  for (int i = 0; i < numberOfRelayButtons; i++) {
    // if this relay has multiple buttons, load only first
    if (relayMultiButtons[i].firstButton == -1 || relayMultiButtons[i].firstButton == i) {
      // Then set relay pins in output mode
      pinMode(myRelayButtons[i].relay, OUTPUT);
      // Set relay to last known state (using eeprom storage)
      uint8_t isTurnedOn = loadState(i); // 1 - true, 0 - false
      uint8_t state = isTurnedOn ? myRelayButtons[i].relayTrigger : ! myRelayButtons[i].relayTrigger;
      digitalWrite(myRelayButtons[i].relay, state);
    }
  }
}

// executed AFTER mysensors has been initialised
void setup() {
  Serial.begin(115200);
  // Setup locally attached sensors
  delay(5000);
  // Setup Relays
  for(int i = 0; i < numberOfRelayButtons; i++) {
    // if this relay has multiple buttons, send only first
    if (relayMultiButtons[i].firstButton == -1 || relayMultiButtons[i].firstButton == i) {
      msgs[i] = MyMessage(i, V_LIGHT);
      send(msgs[i].set(loadState(i))); // send current state
    }
  }
  // Setup buttons
  for(int i = 0; i < numberOfRelayButtons; i++) {
    pinMode(myRelayButtons[i].button, INPUT_PULLUP);
    // After setting up the button, setup debouncer.
    myButtonDebouncer[i] = Bounce();
    myButtonDebouncer[i].attach(myRelayButtons[i].button);
    myButtonDebouncer[i].interval(50);
  }
  //presentation();
}

void loop() {
  for(int i = 0; i < numberOfRelayButtons; i++) {
    if (myButtonDebouncer[i].update()) {
      int buttonState = myButtonDebouncer[i].read();
#ifdef MY_DEBUG
      Serial.print("Button ");
      Serial.print(i);
      Serial.print(" changed to: ");
      Serial.println(buttonState);
#endif
      
      // If button type is BI_STABLE, any change will toggle relay state
      // For MONO_STABLE, button must be pushed and released
      if (myRelayButtons[i].buttonType == BI_STABLE || buttonState == LOW) {
        uint8_t isTurnedOn = ! loadState(i); // 1 - true, 0 - false
        uint8_t newState = isTurnedOn ? myRelayButtons[i].relayTrigger : ! myRelayButtons[i].relayTrigger;
        saveState(i, isTurnedOn);
        digitalWrite(myRelayButtons[i].relay, newState);
        send(msgs[i].set(isTurnedOn));
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
void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Multi Relay", "1.1");
	
	// Register every relay as separate sensor
  for (int i = 0; i < numberOfRelayButtons; i++) {
    // if this relay has multiple buttons, load only once
    if (relayMultiButtons[i].firstButton == -1 || relayMultiButtons[i].firstButton == i) {
      // Register all sensors to gw (they will be created as child devices)
      present(i, S_LIGHT);
    }
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
    uint8_t newState = isTurnedOn ? myRelayButtons[message.sensor].relayTrigger : ! myRelayButtons[message.sensor].relayTrigger;
    // Change relay state
    digitalWrite(myRelayButtons[message.sensor].relay, newState);
    // Store state in eeprom if changed
    if (loadState(message.sensor) != isTurnedOn) {
      saveState(message.sensor, isTurnedOn);
    }
#ifdef MY_DEBUG
    // Write some debug info
    Serial.print("Incoming change for sensor: " + message.sensor);
    Serial.println(", New status: " + isTurnedOn);
#endif
  }
}

