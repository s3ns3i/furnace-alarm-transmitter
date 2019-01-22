// TRANSMITTER

#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver;

const char *msg = "24";

void setup()
{
    Serial.begin(4800);   // Debugging only
    if (!driver.init())
         Serial.println("init failed");
}

void loop()
{
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    Serial.println("Sent package");
    delay(1000);
}
