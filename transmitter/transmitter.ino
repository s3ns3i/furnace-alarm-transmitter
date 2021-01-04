// TRANSMITTER

#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <string.h>
#include <LiquidCrystal.h>

#define CRITICAL "CRT"
#define TOO_HOT "HOT"
#define TOO_COLD "COL"
#define OK "OK"
#define TOO_HOT_LIMIT_UP 85
#define TOO_HOT_LIMIT_DOWN 40
#define TOO_COLD_LIMIT_UP 75
#define TOO_COLD_LIMIT_DOWN 10
#define TOO_HOT_ADDR 0
#define TOO_COLD_ADDR 1
#define THERMOMETER_INDEX 0
#define SENDING_INTERVAL 1000
// #define TEMP_TOO_HOT 25
// #define TEMP_TOO_COLD 24
#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5

const byte ONEWIRE_PIN = 11;

OneWire onewire(ONEWIRE_PIN);
DallasTemperature sensors(&onewire);
RH_ASK driver(2000, 12, 13, 2);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

byte thermometerAddress[8];
float currentTemperature = 0.0f;
char msg[10] = "";
int lcd_key = 0;
int adc_key_in = 0;
int too_hot = 25; // 85
int too_cold = 24; // 40
int critical = 25; // 90
bool TOO_HOT_SELECT = false;
bool TOO_COLD_SELECT = false;
long previousTime = 0L;

void setup()
{
  Serial.begin(9600); // Debugging only
  initTransmitter();
  findThermometerAddress();
  turnOnDisplay();
}

void initTransmitter()
{
  if (!driver.init())
  {
    Serial.println("init failed");
  }
}

void findThermometerAddress()
{
  onewire.reset_search();
  while (onewire.search(thermometerAddress))
  {
    if (thermometerAddress[0] != 0x28)
      continue;

    if (OneWire::crc8(thermometerAddress, 7) != thermometerAddress[7])
    {
      Serial.println(F("Błędny adres, sprawdz polaczenia"));
      break;
    }

    for (byte i = 0; i < 8; i++)
    {
      Serial.print(F("0x"));
      Serial.print(thermometerAddress[i], HEX);

      if (i < 7)
        Serial.print(F(", "));
    }
    Serial.println();
  }
}

void turnOnDisplay()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
}

void loop()
{
  // loadData();
  temperatureReadingCycle();
  printScreen();
  read_LCD_buttons();
  readButtonInput();
  // delay(1000);
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

void temperatureReadingCycle()
{
  long currentTime = millis();
  long interval = currentTime - previousTime;

  if (interval >= SENDING_INTERVAL)
  {
    previousTime = currentTime;
    readTemperature();
    printTemperature();
    sendData();
  }
}

void readTemperature()
{
  sensors.requestTemperatures();
  interpretTemperature();
}

void interpretTemperature()
{
  msg[0] = '\0';
  currentTemperature = sensors.getTempCByIndex(THERMOMETER_INDEX);
  char tempString[6];

  dtostrf(currentTemperature, 5, 2, tempString);
  if (currentTemperature <= (float)too_cold)
  {
    strcat(msg, TOO_COLD);
    msg[3] = ':';
    msg[4] = '\0';
  }
  else if(currentTemperature >= (float)critical)
  {
    strcat(msg, CRITICAL);
    msg[3] = ':';
    msg[4] = '\0';
  }
  else if (currentTemperature >= (float)too_hot)
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

void printScreen()
{
  if (TOO_HOT_SELECT == true)
  {
    lcd.setCursor(0, 1);
    lcd.print("HOT");

    lcd.setCursor(11, 1);
    lcd.print(too_hot);
  }
  else if (TOO_COLD_SELECT == true)
  {
    lcd.setCursor(0, 1);
    lcd.print("COLD");

    lcd.setCursor(6, 1);
    lcd.print(too_cold);
  }
}

void printTemperature()
{
  lcd.setCursor(5, 0);
  lcd.print("      ");
  lcd.setCursor(5, 0);
  lcd.print(currentTemperature);
}

void readButtonInput()
{
  if (lcd_key == btnUP && TOO_HOT_SELECT == true)
  {
    clearRow(1);
    ++too_hot;
    if (too_hot > TOO_HOT_LIMIT_UP)
    {
      too_hot = TOO_HOT_LIMIT_UP;
    }
  }
  else if (lcd_key == btnUP && TOO_COLD_SELECT == true)
  {
    clearRow(1);
    ++too_cold;
    if (too_cold > TOO_COLD_LIMIT_UP)
    {
      too_cold = TOO_COLD_LIMIT_UP;
    }
  }
  else if (lcd_key == btnDOWN && TOO_HOT_SELECT == true)
  {
    clearRow(1);
    --too_hot;
    if (too_hot < TOO_HOT_LIMIT_DOWN)
    {
      too_hot = TOO_HOT_LIMIT_DOWN;
    }
  }
  else if (lcd_key == btnDOWN && TOO_COLD_SELECT == true)
  {
    clearRow(1);
    --too_cold;
    if (too_cold < TOO_COLD_LIMIT_DOWN)
    {
      too_cold = TOO_COLD_LIMIT_DOWN;
    }
  }
  else if (lcd_key == btnSELECT)
  {
    if (TOO_COLD_SELECT == true)
    {
      lcd.setCursor(0, 1);
      lcd.print("    ");
      TOO_HOT_SELECT = false;
      TOO_COLD_SELECT = false;
    }
    else if (TOO_HOT_SELECT == false)
    {
      lcd.setCursor(0, 1);
      lcd.print("HOT ");
      TOO_HOT_SELECT = true;
      TOO_COLD_SELECT = false;
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("COLD");
      TOO_HOT_SELECT = false;
      TOO_COLD_SELECT = true;
    }
  }
}

void read_LCD_buttons()
{
  adc_key_in = analogRead(0);

  if (adc_key_in > 1000)
  {
    lcd_key = btnNONE;
    return;
  }

  if (adc_key_in < 50)
  {
    Serial.println("Pressed right");
    lcd_key = btnRIGHT;
    return;
  }
  if (adc_key_in < 250)
  {
    Serial.println("Pressed up");
    lcd_key = btnUP;
    return;
  }
  if (adc_key_in < 450)
  {
    Serial.println("Pressed down");
    lcd_key = btnDOWN;
    return;
  }
  if (adc_key_in < 650)
  {
    Serial.println("Pressed left");
    lcd_key = btnLEFT;
    return;
  }
  if (adc_key_in < 850)
  {
    Serial.println("Pressed select");
    lcd_key = btnSELECT;
    return;
  }
}

void clearRow(int row)
{
  lcd.setCursor(0, row);
  lcd.print("                ");
}

/*
 * What it should do:
 * 1. Detect if temperature has gone too low and alarm        CHECK
 *    itself and receiver about it
 * 2. Do the same when temperature has gone too high          CHECK
 * 3. Detect if external power-source has been disconnected   TODO
 *    and alarm about itself and receiver about it
 * 4. Show current temperature                                CHECK
 * 5. Allow to change minimum and maximum temperature         IN PROGRESS
 * 6. Stop alarming if temperature has gone into threshold    NOT IMPLEMENTING
 *    again, or if snooze button has been pressed.
 * 7. If temperature is above threshold and device is         
 *    snoozed, then if it goes another 5 degrees up, it
 *    should start alarming again
 * 8. Do the same when temperature is below threshold
 * 
 */