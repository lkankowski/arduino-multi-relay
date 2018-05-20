# About
Arduino program to handle relays and control them using switches.
Single pair of relay and switch are configured in single line.
Buttons has debouncing and support for mono- and bi-stable switches.
There is support for multiple switches for single relay.

**ONLY VERSION 1.0 is tested in real life! Current MASTER is still experimantal!**


# Sample configurations
## Simple mono-stable switch and relay pair
Relay is connected to pin 12, button is connected to pin 11, relay is triggered to ON using LOW state, button is mono-stable
```
RelayButton myRelayButtons[] = {
    {12, 11, LOW, MONO_STABLE}
};
```

## Simple bi-stable switch and relay pair
Relay is connected to pin 12, button is connected to pin 11, relay is triggered to ON using HIGH state, button is bi-stable
```
RelayButton myRelayButtons[] = {
    {12, 11, HIGH, BI_STABLE}
};
```

## Simple two mono-stable switch for one relay
Relay is connected to pin 12, buttons are connected to pins 11 and 10, relay is triggered to ON using HIGH state, buttons are mono-stable
```
RelayButton myRelayButtons[] = {
    {12, 11, HIGH, MONO_STABLE}
    {12, 10, HIGH, MONO_STABLE}
};
```
