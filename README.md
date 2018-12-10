# About
Arduino program to handle relays and control them using switches.
Single pair of relay and switch are configured in single line.
Buttons has debouncing and support for mono- and bi-stable switches.
There is support for multiple switches for single relay.

I left configuration variable _myRelayButtons[]_ which is used at my house, so before use you HAVE TO change it.

# Configuration
## Main config
```
RelayButton myRelayButtons[] = {
    {sensor ID, relay pin, button pin, relay options, button type, relay description}
};
```
Params description:
* sensor ID - sensor ID for Domoticz, must be unique (can be -1 when it is not reported to Domoticz)
* relay pin - pin connected to the relay. Expander is supported (see details inseparate section)
* button pin - pin connected to the button. There can be multiple buttons to the same relay. "<0" for virtual buttons (only available in Domoticz). There is no support for Expander because of de-bouncing.
* relay options - combined with '|' operator:
** trigger level - RELAY_TRIGGER_LOW or RELAY_TRIGGER_HIGH
** startup state - optional, RELAY_STARTUP_ON or RELAY_STARTUP_OFF
* button type:
** MONO_STABLE - GND connected to the button pin
** BI_STABLE - state change from LOW to HIGH and HIGH to LOW, used with mechanical and touch buttons
** DING_DONG - doorbell button, relay is triggered only when button is pushed (experimental)
* relay description - visible in Domoticz/Home Assistant, helps initial configuration (can be null - "")

In my case, I have a documentation of whole electricity of my house in Google Calc, and I have a formula to generate this configuration.

# Debugging
```
#define MY_DEBUG
```
Just comment this on production.

# Sample configurations
## Simple mono-stable switch and relay pair
Relay is connected to pin 12, button is connected to pin 11, relay is triggered to ON using LOW state, button is mono-stable
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, LOW, MONO_STABLE, ""}
};
```

## Simple bi-stable switch and relay pair
Relay is connected to pin 12, button is connected to pin 11, relay is triggered to ON using HIGH state, button is bi-stable
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, HIGH, BI_STABLE, ""}
};
```

## Simple two mono-stable switch for one relay
Relay is connected to pin 12, buttons are connected to pins 11 and 10, relay is triggered to ON using HIGH state, buttons are mono-stable
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, HIGH, MONO_STABLE, ""}
    {2, 12, 10, HIGH, MONO_STABLE, ""}
};
```

# Expander PCF8574 Support
To use expander PCF8574 you have to uncomment this line:
```
#define USE_EXPANDER
```
From now you can use expander pins in configuration _myRelayButtons[]_. To recognize expander pin, numbers start from 0x0100 and have special meaning:
* first byte - expander number (starts from 1)
* second byte - pin number
In example - "0x0100" means pin 0 on first expander

To simplify using expanders, there are some constants _Eab_ where:
* a - expander number (starts from 0)
* b - pin on expander [0-f]
```
#define E00 0x0100
```

To add more expanders, at this moment you need following tasks:
* add additional instances of expander -   PCF8574 expander1; PCF8574 expander1; ...
* add more constants _Eab_
* initialize all expanders - expander0.begin(0x20); expander1.begin(0x20); ...
* change code in before() function - #ifdef USE_EXPANDER ... if ( myRelayButtons[i].relay & 0x0200 ) {... expander1.pinMode ...
* change code in changeRelayState() function - #ifdef USE_EXPANDER ... if ( myRelayButtons[i].relay & 0x0200 ) {... expander1.digitalWrite( ...
