// Enable debug prints to serial monitor
#define USE_EXPANDER
#define MY_DEBUG

#define MY_GATEWAY_SERIAL

#include <MySensors.h>
#include <Bounce2.h>
#ifdef USE_EXPANDER
  #include <Wire.h>    // Required for I2C communication
  #include "PCF8574.h" // Required for PCF8574
  PCF8574 expander0;
  // Expander constants
  #define E00 0x0100
  #define E01 0x0101
  #define E02 0x0102
  #define E03 0x0103
  #define E04 0x0104
  #define E05 0x0105
  #define E06 0x0106
  #define E07 0x0107
#endif

// No Button Constant
#define NOB -1
#define MULTI_RELAY_VERSION 9
#define RELAY_STATE_STORAGE 1

const uint8_t RELAY_TRIGGER_LOW  = 0;
const uint8_t RELAY_TRIGGER_HIGH = 1;
const uint8_t RELAY_STARTUP_ON   = 2;
const uint8_t RELAY_STARTUP_OFF  = 4;

enum ButtonType {
  MONO_STABLE = 0,
  BI_STABLE = 1,
  DING_DONG = 2 // HIGH state immediatly after push, LOW state after release
};

typedef struct {
  int sensorId;
  int relay;
  int button;
  uint8_t relayOptions;
  ButtonType buttonType;
  const char * relayDescription;
} RelayButton;

// CONFIGURE ONLY THIS ARRAY!
// Row params: sensor ID - sensor ID for Domoticz
//             relay pin - <0 for virtual buttons (only available in Domoticz); no support for Expander
//             button pin - Expander supported
//             relay options - [RELAY_TRIGGER_LOW|RELAY_TRIGGER_HIGH] {RELAY_STARTUP_ON|RELAY_STARTUP_OFF}
//             button type - [MONO_STABLE|BI_STABLE|DING_DONG]
RelayButton myRelayButtons[] = {
  {0, 2, A0, RELAY_TRIGGER_LOW, MONO_STABLE, "Ł2 - kinkiet [C10]"},  // WŁ: Ł2
  {1, 16, A1, RELAY_TRIGGER_LOW, BI_STABLE, "Salon 2 [A9]"},  // WŁ: Salon 2
  {2, 15, A2, RELAY_TRIGGER_LOW, BI_STABLE, "Salon 1 [A10]"},  // WŁ: Salon 1
  {3, E01, A3, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON, BI_STABLE, "Halogen - Taras [B8]"},  // WŁ: Taras
  {4, 22, A4, RELAY_TRIGGER_LOW, BI_STABLE, "Kuchnia [B2]"},  // WŁ: Kuchnia 1
  {5, 23, A5, RELAY_TRIGGER_LOW, BI_STABLE, "Kuchnia - Kinkiet [B3]"},  // WŁ: Kuchnia 2
  {6, 28, A6, RELAY_TRIGGER_LOW, BI_STABLE, "Jadalnia 2 [A4]"},  // WŁ: Hall I/Jadalnia 3
  {7, 30, A7, RELAY_TRIGGER_LOW, BI_STABLE, "Jadalnia 1 [A6]"},  // WŁ: Hall I/Jadalnia 2
  {8, 31, A8, RELAY_TRIGGER_LOW, MONO_STABLE, "Garaż [A7]"},  // WŁ: Kotłownia/Garaż
  {-1, 31, A9, RELAY_TRIGGER_LOW, MONO_STABLE, "Garaż [A7]"},  // WŁ: Garaż
  {10, 14, A10, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON, BI_STABLE, "Halogen - wejście [B4]"},  // WŁ: Drzwi wejściowe
  {11, E07, A11, RELAY_TRIGGER_LOW, DING_DONG, "Dzwonek [?]"},  // WŁ: Dzwonek
  {12, 29, A12, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 1 [A5]"},  // WŁ: Hall I/Jadalnia 1
  {-1, 29, A13, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 1 [A5]"},  // WŁ: Hall I/Wiatrołap
  {14, 32, A14, RELAY_TRIGGER_LOW, BI_STABLE, "Wiatrołap [A8]"},  // WŁ: Wiatrołap/Hall I
  {15, 19, A15, RELAY_TRIGGER_LOW, MONO_STABLE, "Kotłownia [B1]"},  // WŁ: Kotłownia/Hall I
  {16, 24, 53, RELAY_TRIGGER_LOW, BI_STABLE, "Ł1 - Taśma LED [?]"},  // WŁ: Hall I/Ł1 1
  {17, 17, 52, RELAY_TRIGGER_LOW, MONO_STABLE, "Ł1 - Kinkiet [A11]"},  // WŁ: Ł1
  {18, 18, 51, RELAY_TRIGGER_LOW, BI_STABLE, "Ł1 [A12]"},  // WŁ: Hall I/Ł1 2
  {19, 6, 50, RELAY_TRIGGER_LOW, BI_STABLE, "Klatka Schodowa [B7]"},  // WŁ: Hall I/Schody 1
  {-1, 29, 49, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 1 [A5]"},  // WŁ: Hall I/Schody 2
  {21, 26, 48, RELAY_TRIGGER_LOW, BI_STABLE, "Gabinet [A2]"},  // WŁ: Gabinet
  {22, 7, 47, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 2 [B5]"},  // WŁ: Hall II/Schody 1
  {-1, 6, 46, RELAY_TRIGGER_LOW, BI_STABLE, "Klatka Schodowa [B7]"},  // WŁ: Hall II/Schody 2
  {24, 10, 45, RELAY_TRIGGER_LOW, BI_STABLE, "Garderoba [C12]"},  // WŁ: Garderoba
  {25, 4, 44, RELAY_TRIGGER_LOW, MONO_STABLE, "Pok. nad kuchnią 2 [B10]"},  // WŁ: Pok. nad kuchnią 2
  {26, 5, 43, RELAY_TRIGGER_LOW, MONO_STABLE, "Pok. nad kuchnią 1 [B9]"},  // WŁ: Pok. nad kuchnią 1
  {27, 8, 42, RELAY_TRIGGER_LOW, MONO_STABLE, "Pok. nad salonem 2 [B12]"},  // WŁ: Pok. nad salonem 2
  {28, 9, 41, RELAY_TRIGGER_LOW, MONO_STABLE, "Pok. nad salonem 1 [B11]"},  // WŁ: Pok. nad salonem 1
  {29, 3, 40, RELAY_TRIGGER_LOW, MONO_STABLE, "Ł2 [C7]"},  // WŁ: Hall II/Ł2 1
  {30, E03, 39, RELAY_TRIGGER_LOW, MONO_STABLE, "Ł2 - Taśma LED [?]"},  // WŁ: Hall II/Ł2 2
  {-1, 7, 38, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 2 [B5]"},  // WŁ: Hall II/Sypialnia
  {32, 11, 37, RELAY_TRIGGER_LOW, BI_STABLE, "Sypialnia 2 [C9]"},  // WŁ: Sypialnia 2
  {33, 12, 36, RELAY_TRIGGER_LOW, BI_STABLE, "Sypialnia 1 [C8]"},  // WŁ: Sypialnia 1
  {34, 25, -1, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON, MONO_STABLE, "Halogen - Garaż [A1]"},  // WŁ: Virtual Button 1
  {35, 27, -2, RELAY_TRIGGER_LOW, MONO_STABLE, "Ł1 - Wentylator [A3]"},  // WŁ: Virtual Button 2
  {36, E02, -3, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON, MONO_STABLE, "Halogen - wschód [B6]"},  // WŁ: Virtual Button 3
  {37, E04, -4, RELAY_TRIGGER_LOW, MONO_STABLE, "Lampki schodowe [C6]"},  // WŁ: Virtual Button 4
  {38, E05, -5, RELAY_TRIGGER_LOW, MONO_STABLE, "Lampki podłogowe I [C4]"},  // WŁ: Virtual Button 5
  {39, E06, -6, RELAY_TRIGGER_LOW, MONO_STABLE, "Lampki podłogowe II [C2]"},  // WŁ: Virtual Button 6
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
  Serial.begin(115200);
  
  #ifdef USE_EXPANDER
    /* Start I2C bus and PCF8574 instance */
    expander0.begin(0x20);
  #endif
  
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
  
  // if version has changed, reset state of all relays
  int versionChangeResetState = (MULTI_RELAY_VERSION == loadState(0) ) ? 0 : 1;
  
  for (int i = 0; i < numberOfRelayButtons; i++) {
    // if this relay has multiple buttons, load only first
    if (relayMultiButtons[i].firstButton == -1 || relayMultiButtons[i].firstButton == i) {
      // Then set relay pins in output mode
      #ifdef USE_EXPANDER
        if ( myRelayButtons[i].relay >= 0x0100 ) {
          // EXPANDER
          expander0.pinMode(myRelayButtons[i].relay & 0xf, OUTPUT);
        } else {
      #endif
          pinMode(myRelayButtons[i].relay, OUTPUT);
      #ifdef USE_EXPANDER
        }
      #endif
      
      uint8_t isTurnedOn = 0;
      uint8_t useEeprom = 1;
      uint8_t relayTrigger = myRelayButtons[i].relayOptions & RELAY_TRIGGER_HIGH;
      
      if (myRelayButtons[i].relayOptions & RELAY_STARTUP_ON) {
        isTurnedOn = 1;
        useEeprom = 0;
      } else if (myRelayButtons[i].relayOptions & RELAY_STARTUP_OFF) {
        useEeprom = 0;
      }
      if (versionChangeResetState && useEeprom) {
        saveRelayState(i, 0);
        useEeprom = 0;
      }

      if (useEeprom) {
        // Set relay to last known state (using eeprom storage)
        isTurnedOn = loadRelayState(i); // 1 - true, 0 - false
      }
      uint8_t state = isTurnedOn ? relayTrigger : ! relayTrigger;
      digitalWrite(myRelayButtons[i].relay, state);
    }
  }
  if (versionChangeResetState) {
    // version has changed, so store new version in eeporom
    saveState(0, MULTI_RELAY_VERSION);
  }
}

// executed AFTER mysensors has been initialised
void setup() {
  for(int i = 0; i < numberOfRelayButtons; i++) {
    if (myRelayButtons[i].button >= 0) {
      // No Expander support for buttons (de-bouncing)
      pinMode(myRelayButtons[i].button, INPUT_PULLUP); // HIGH state when button is not pushed
    }
  }
  // Setup locally attached sensors
  delay(5000);
  // Setup Relays
  for(int i = 0; i < numberOfRelayButtons; i++) {
    // if this relay has multiple buttons, send only first
    if (relayMultiButtons[i].firstButton == -1 || relayMultiButtons[i].firstButton == i) {
      msgs[i] = MyMessage(myRelayButtons[i].sensorId, V_LIGHT);
      uint8_t relayState;
      if (myRelayButtons[i].relayOptions & RELAY_STARTUP_ON) {
        relayState = 1;
      } else if (myRelayButtons[i].relayOptions & RELAY_STARTUP_OFF) {
        relayState = 0;
      } else {
        relayState = loadRelayState(i);
      }
      send(msgs[i].set(relayState)); // send current state
    }
  }
  // Setup buttons
  for(int i = 0; i < numberOfRelayButtons; i++) {
    if (myRelayButtons[i].button >= 0) {
      // setup debouncer
      myButtonDebouncer[i] = Bounce();
      myButtonDebouncer[i].attach(myRelayButtons[i].button);
      myButtonDebouncer[i].interval(50);
    }
  }
}

void loop() {
  for(int i = 0; i < numberOfRelayButtons; i++) {
    if (myRelayButtons[i].button >= 0 && myButtonDebouncer[i].update()) {
      int buttonState = myButtonDebouncer[i].read();
      #ifdef MY_DEBUG
        Serial.print("# Button ");
        Serial.print(i);
        Serial.print(" changed to: ");
        Serial.println(buttonState);
      #endif
      
      int nextButton = (relayMultiButtons[i].firstButton == -1) ? i : relayMultiButtons[i].firstButton;
      uint8_t relayTrigger = myRelayButtons[nextButton].relayOptions & RELAY_TRIGGER_HIGH;
      uint8_t useEeprom = ((myRelayButtons[nextButton].relayOptions & (RELAY_STARTUP_ON | RELAY_STARTUP_OFF)) == 0);
      
      if (myRelayButtons[i].buttonType == DING_DONG) {
        if (buttonState == LOW) { // button pressed
          digitalWrite(myRelayButtons[nextButton].relay, relayTrigger);
          send(msgs[nextButton].set(1));
        } else { // button released
          digitalWrite(myRelayButtons[nextButton].relay, ! relayTrigger);
          send(msgs[nextButton].set(0));
        }
      } else if (myRelayButtons[i].buttonType == BI_STABLE || buttonState == HIGH) {
        // If button type is BI_STABLE, any change will toggle relay state
        // For MONO_STABLE, button must be pushed and released (HIGH)
        uint8_t isTurnedOn = ! loadRelayState(nextButton); // 1 - true, 0 - false
        uint8_t newState = isTurnedOn ? relayTrigger : ! relayTrigger;
        digitalWrite(myRelayButtons[nextButton].relay, newState);
        send(msgs[nextButton].set(isTurnedOn));
        
        // update state in eeprom for all buttons
        do {
          if (useEeprom) {
            saveRelayState(nextButton, isTurnedOn);
          }
          nextButton = relayMultiButtons[nextButton].nextButton;
        } while (nextButton != -1);
      }
    }
  }
}



// MySensors - Presentation
// Your sensor must first present itself to the controller.
// The presentation is a hint to allow controller prepare for the sensor data that eventually will come.
// Executed after "before()" and before "setup()" in: _begin (MySensorsCore.cpp) > gatewayTransportInit() > presentNode()
void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Multi Relay", "1.2");
	
	// Register every relay as separate sensor
  for (int i = 0; i < numberOfRelayButtons; i++) {
    // if this relay has multiple buttons, register only first
    if (relayMultiButtons[i].firstButton == -1 || relayMultiButtons[i].firstButton == i) {
      // Register all sensors to gw (they will be created as child devices)
      // void present(uint8_t childSensorId, uint8_t sensorType, const char *description, bool ack);
      //   childSensorId - The unique child id you want to choose for the sensor connected to this Arduino. Range 0-254.
      //   sensorType - The sensor type you want to create.
      //   description An optional textual description of the attached sensor.
      //   ack - Set this to true if you want destination node to send ack back to this node. Default is not to request any ack.
      present(i, S_BINARY, myRelayButtons[i].relayDescription);
    }
  }
}


// MySensors - Handling incoming messages
// Nodes that expects incoming data, such as an actuator or repeating nodes,
// must implement the receive() - function to handle the incoming messages.
// Do not sleep a node where you expect incoming data or you will lose messages.
void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_STATUS) {
    uint8_t isTurnedOn = message.getBool(); // 1 - true, 0 - false
    uint8_t relayTrigger = myRelayButtons[message.sensor].relayOptions & RELAY_TRIGGER_HIGH;
    uint8_t useEeprom = ((myRelayButtons[message.sensor].relayOptions & (RELAY_STARTUP_ON | RELAY_STARTUP_OFF)) == 0);
    uint8_t newState = isTurnedOn ? relayTrigger : ! relayTrigger;
    // Change relay state
    digitalWrite(myRelayButtons[message.sensor].relay, newState);
    // Store state in eeprom if changed
    if (useEeprom && loadRelayState(message.sensor) != isTurnedOn) {
      saveRelayState(message.sensor, isTurnedOn);
    }
    send(msgs[message.sensor].set(isTurnedOn)); // support for OPTIMISTIC=FALSE (Home Asistant)
    #ifdef MY_DEBUG
      // Write some debug info
      Serial.print("# Incoming change for sensor: " + message.sensor);
      Serial.println(", New status: " + isTurnedOn);
    #endif
  }
}

uint8_t loadRelayState(int relayNum) {
  uint8_t relayState = loadState(RELAY_STATE_STORAGE + relayNum);
  #ifdef MY_DEBUG
    Serial.print("# loadRelayState: ");
    Serial.print(relayNum);
    Serial.print("(byte ");
    Serial.print(RELAY_STATE_STORAGE + relayNum);
    Serial.print(") = ");
    Serial.println(relayState);
  #endif
  return(relayState);
}

void saveRelayState(int relayNum, uint8_t state) {
  saveState(RELAY_STATE_STORAGE + relayNum, state);
}
