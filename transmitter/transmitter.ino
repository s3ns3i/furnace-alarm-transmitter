// TRANSMITTER

#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver;

const char *msg = "HOT:90.00";

void setup()
{
    Serial.begin(4800); // Debugging only
    if (!driver.init())
        Serial.println("init failed");
}

void loop()
{
    sendData();
}

void sendData()
{
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    Serial.println("Sent package");
    delay(1000);
}

/*
 * What it should do:
 * 1. Detect if temperature has gone too low and alarm
 *    itself and receiver about it
 * 2. Do the same when temperature has gone too high
 * 3. Detect if external power-source has been disconnected
 *    and alarm about itself and receiver about it
 * 4. Show current temperature
 * 5. Allow to change minimum and maximum temperature
 * 6. Stop alarming if temperature has gone into threshold
 *    again, or if snooze button has been pressed.
 * 7. If temperature is above threshold and device is
 *    snoozed, then if it goes another 5 degrees up, it
 *    should start alarming again
 * 8. Do the same when temperature is below threshold
 * 
 */