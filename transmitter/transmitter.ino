// TRANSMITTER

#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <string.h>

#define TOO_HOT "HOT"
#define TOO_COLD "COL"
#define OK "OK"
#define TOO_HOT_ADDR 0
#define TOO_COLD_ADDR 1
#define THERMOMETER_INDEX 0
#define TEMP_TOO_HOT 25
#define TEMP_TOO_COLD 24

const byte ONEWIRE_PIN = 11;

OneWire onewire(ONEWIRE_PIN);
DallasTemperature sensors(&onewire);
RH_ASK driver;

byte thermometerAddress[8];
char msg[10] = "";
bool buttonPressed = false;

void setup()
{
    Serial.begin(9600); // Debugging only
    if (!driver.init())
    {
        Serial.println("init failed");
    }
    findThermometerAddress();
}

void findThermometerAddress()
{
  onewire.reset_search();
  while(onewire.search(thermometerAddress))
  {
    if (thermometerAddress[0] != 0x28)
      continue;

    if (OneWire::crc8(thermometerAddress, 7) != thermometerAddress[7])
    {
      Serial.println(F("Błędny adres, sprawdz polaczenia"));
      break;
    }

    for (byte i=0; i<8; i++)
    {
      Serial.print(F("0x"));
      Serial.print(thermometerAddress[i], HEX);

      if (i < 7)
        Serial.print(F(", "));
    }
    Serial.println();
  }
}

void loop()
{
    // loadData();
    readTemperature();
    interpretTemperature();
    sendData();
    delay(1000);
}

void loadData()
{
    Serial.println("loadData() - Not yet implemented.");
}

uint8_t *loadData(const char *KEY)
{
    Serial.println("loadData(const char* KEY) - Not yet implemented.");
    // return "";
}

void readTemperature()
{
    sensors.requestTemperatures();
}

void interpretTemperature()
{
    msg[0] = '\0';
    float currentTemperature = sensors.getTempCByIndex(THERMOMETER_INDEX);
    char tempString[6];

    dtostrf(currentTemperature, 5, 2, tempString);
    // Serial.print("Current temperature: ");
    // Serial.println(tempString);
    if(currentTemperature <= TEMP_TOO_COLD)
    {
        strcat(msg, TOO_COLD);
        msg[3] = ':';
        msg[4] = '\0';
    }
    else if(currentTemperature >= TEMP_TOO_HOT)
    {
        strcat(msg, TOO_HOT);
        msg[3] = ':';
        msg[4] = '\0';
    }
    else
    {
        strcat(msg, OK);
        msg[2] = ':';
        msg[3] = '\0';
    }
    strcat(msg, tempString);

    Serial.println(msg);
}

void sendData()
{
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    Serial.println("Sent package");
}

void saveData(uint8_t *data)
{
    Serial.println("Saving data to EEPROM");
    uint8_t key[4];
    uint8_t value[8];
    //   readData(data, key, value);
    Serial.print("Int value: ");
    Serial.println((char *)value);
    EEPROM.write(TOO_HOT_ADDR, atoi((char *)value));
    int readValue = EEPROM.read(TOO_HOT_ADDR);
    Serial.print("Read value: ");
    Serial.println(readValue);
    // uint8_t readValue =
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