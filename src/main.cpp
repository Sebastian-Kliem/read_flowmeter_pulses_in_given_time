#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <limits.h>
#include <SPI.h>

// Defines for Pins
const int flowMeterPin = 2;
const int valve = 22;
const int buttonPin1Second = 8;
const int buttonPin3Second = 9;
const int buttonPin10Second = 10;
const int buttonPin100Second = 11;

// define variables
int pulses = 0;
volatile bool isRunning = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;

// Defines for Display
int i2cAddress = 0x3F;
int lcdColumns = 16;
int lcdRows = 2;
/**
 * @brief Initializes the LCD display with the specified I2C address, number of columns, and rows.
 *
 * This function sets up the LiquidCrystal_I2C library with the provided parameters. It configures the
 * LCD display to communicate with the Arduino using the I2C protocol.
 *
 * @param i2cAddress The I2C address of the LCD display.
 * @param lcdColumns The number of columns on the LCD display.
 * @param lcdRows The number of rows on the LCD display.
 *
 * @return void
 */
LiquidCrystal_I2C lcd(i2cAddress, lcdColumns, lcdRows);

/**
 * Count the pulses from the flow meter
 * Triggerd by the intterrupt
 */
void countPulse()
{
  pulses++;
}

/**
 * Clear the LCD-Display line with spaces
 *
 * @param line numberof line (0-indexed) on the LCD-Display to clear.
 */
void clearLine(int line)
{
  lcd.setCursor(0, line);
  for (int i = 0; i < lcdColumns; i++)
  {
    lcd.print(" ");
  }
}

/**
 * @brief Writes a given string to a specified line of the LCD display.
 *
 * This function clears the specified line of the LCD display, sets the cursor to the beginning of the line,
 * and then prints the given string. If no line is specified, the string is written to the first line.
 *
 * @param string_to_write The string to be written to the LCD display.
 * @param line The line number on the LCD display (0-indexed). Default value is 0.
 *
 * @return void
 */
void writeToDisplay(const String string_to_write, const int line = 0)
{
  clearLine(line);
  lcd.setCursor(0, line);
  lcd.print(string_to_write);
}

/**
 * @brief Initializes the Arduino setup.
 *
 * This function sets up the serial communication, initializes the LCD display, configures the pins,
 * attaches an interrupt to the flow meter pin, and sets the valve pin to HIGH.
 *
 * @return void
 */
void setup()
{
  // init serial monitor
  Serial.begin(9600);

  // display
  lcd.init();
  lcd.begin(lcdColumns, lcdRows);
  lcd.backlight();
  lcd.setBacklight(HIGH);

  // Config Pins
  pinMode(flowMeterPin, INPUT_PULLUP);
  pinMode(valve, OUTPUT);
  pinMode(buttonPin1Second, INPUT_PULLUP);
  pinMode(buttonPin3Second, INPUT_PULLUP);
  pinMode(buttonPin10Second, INPUT_PULLUP);
  pinMode(buttonPin100Second, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(flowMeterPin), countPulse, FALLING);

  digitalWrite(valve, HIGH);

  writeToDisplay("Ready");
}

/**
 * @brief Runs a full measurement with the valve open for a specified number of seconds.
 *
 * This function measures the flow rate by counting the pulses from a flow meter.
 * The valve is opened for the specified number of seconds, then closed, and the total number of pulses
 * is displayed on the LCD.
 *
 * @param seconds The number of seconds the valve should be open.
 *
 * @return void
 */
void runMessurementFull(unsigned long seconds)
{
  pulses = 0;
  writeToDisplay("Running ");
  writeToDisplay(String(seconds) + " seconds", 1);

  Serial.println("Measurement starts with " + String(seconds) + "s");

  unsigned long startTime = millis();
  unsigned long measuermentTimeMiliSeconds = seconds * 1000;
  unsigned long stoptime = startTime + (measuermentTimeMiliSeconds);
  unsigned long previousMillis = 0;

  digitalWrite(valve, LOW);

  while (millis() <= stoptime)
  {
    // print the seconds of the runtime
    if (millis() / 1000 == previousMillis)
    {
      previousMillis = millis() / 1000;
      Serial.println("Time: " + String(previousMillis) + "s");
    }
  }

  digitalWrite(valve, HIGH);

  Serial.println("Pulses: " + String(pulses));

  writeToDisplay("Pulses");
  writeToDisplay(String(pulses), 1);
}

/**
 * @brief Runs a split measurement with the valve open for a specified number of seconds, repeated 10 times.
 *
 * This function measures the flow rate by counting the pulses from a flow meter.
 * The valve is opened for the specified number of seconds, then closed, and this process is repeated 10 times.
 * The total number of pulses is then displayed on the LCD.
 *
 * @param seconds The number of seconds the valve should be open for each cycle.
 *
 * @return void
 */
void runMessurementSplitted(unsigned int seconds)
{
  pulses = 0;
  writeToDisplay("Running " + String(seconds) + " seconds");

  Serial.println("Splitted measurement starts with 10x " + String(seconds) + "s");

  for (int i = 0; i < 10; i++)
  {
    unsigned long startTime = millis();
    unsigned int cycle = i + 1;

    writeToDisplay("Cycle: " + String(cycle), 1);
    digitalWrite(valve, LOW);

    unsigned long measuermentTimeMiliSeconds = seconds * 1000;
    unsigned long stoptime = startTime + (measuermentTimeMiliSeconds);
    unsigned long previousMillis = 0;

    while (millis() <= stoptime)
    {
      // print the seconds of the runtime
      if (millis() / 1000 == previousMillis)
      {
        previousMillis = millis() / 1000;
        Serial.println("Time: " + String(previousMillis) + "s");
      }
    }

    digitalWrite(valve, HIGH);

    startTime = millis();
    unsigned long waitTime = 0;

    // pause for 2 seconds after each cycle
    while (waitTime < 2000)
    {
      waitTime = millis() - startTime;
    }
  }

  Serial.println("Pulses: " + String(pulses));

  writeToDisplay("Pulses");
  writeToDisplay(String(pulses), 1);
}

void loop()
{
  if (digitalRead(buttonPin1Second) == LOW)
  {
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
      Serial.println("Button 1s pressed");
      runMessurementSplitted(1);
    }
    lastDebounceTime = millis();
  }

  if (digitalRead(buttonPin3Second) == LOW)
  {
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
      Serial.println("Button 3s pressed");
      runMessurementSplitted(3);
    }
    lastDebounceTime = millis();
  }

  if (digitalRead(buttonPin10Second) == LOW)
  {
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
      Serial.println("Button 10s pressed");
      runMessurementFull(10);
    }
    lastDebounceTime = millis();
  }

  if (digitalRead(buttonPin100Second) == LOW)
  {
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
      Serial.println("Button 100s pressed");
      runMessurementFull(100);
    }
    lastDebounceTime = millis();
  }
}
