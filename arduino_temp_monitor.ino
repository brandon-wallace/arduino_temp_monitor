//    ARDUINO TEMPERATURE MONITOR

#include <DS3231.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>



Time now;
char filename[13];
DS3231  rtc(SDA, SCL);
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

#define ONE_WIRE_BUS_PIN 5
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);

DeviceAddress Probe01 = { 0x28, 0xFF, 0xD5, 0xE6, 0x86, 0x16, 0x05, 0x4E }; // First sensor
DeviceAddress Probe02 = { 0x28, 0xFF, 0xB7, 0xE7, 0x86, 0x16, 0x05, 0x53 }; // Second sensor
DeviceAddress Probe03 = { 0x28, 0xFF, 0xF9, 0xAC, 0x8C, 0x16, 0x03, 0xDB }; // Third sensor


void setup()
{
    Serial.begin(9600);

    Serial.println("Initializing i2c LCD");
    lcd.begin(20,4);
 
    Serial.println("Initializing DS3231 real time clock");
    rtc.begin();
    // Uncomment to set date and time.
    //rtc.setDOW(THURSDAY);     // Set Day-of-Week to THURSDAY.
    //rtc.setTime(17, 39, 0);     // Set time to 12:00:00 (24hr format).
    //rtc.setDate(13, 10, 2017);   // Set date to DD MM YYYY.
    now = rtc.getTime();
    String(String(now.year) + String(now.mon) + String(now.date) + ".csv").toCharArray(filename, 13);
    
    // Add a leading zero to day and month digits less that 10.
    String date_var = "";
    int d = now.date;
    if(d < 10) { date_var += '0'; }
    date_var += d;
    
    String month_var = "";
    int m = now.mon;
    if(m < 10) { month_var += '0'; }
    month_var += m;
    
    // Use the date to create the file name of the CSV file.
    String(String(now.year) + String(month_var) + String(date_var) + ".csv").toCharArray(filename, 13);
  
    Serial.println("Initializing DS18B20 Sensors");
    sensors.begin();
    sensors.setResolution(Probe01, 10);
    sensors.setResolution(Probe02, 10);
    sensors.setResolution(Probe03, 10);
    //Serial.print("Sensor1 Resolution: ");
    //Serial.println(sensors.getResolution(Probe01), DEC);
    //sensors.setResolution(Probe02, 10);
    //Serial.print("Sensor2 Resolution: ");
    //Serial.println(sensors.getResolution(Probe02), DEC);
    //sensors.setResolution(Probe03, 10);
    //Serial.print("Sensor2 Resolution: ");
    //Serial.println(sensors.getResolution(Probe03), DEC);
    
    Serial.println("Initializing SD card");
    if (!SD.begin(4)) {
        Serial.println("SD CARD NOT FOUND!");
        return;
        }
    Serial.println("SD Card initialized.");
}


void displayTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    lcd.print("Error");
    Serial.print("Error getting temperature.");
    } else {
    //Serial.print("C: ");
    //Serial.print(tempC);
    //Serial.print(" F: ");
    //Serial.println(DallasTemperature::toFahrenheit(tempC));
    //lcd.print("C: ");
    //lcd.print(tempC);
    lcd.print(" F: ");
    lcd.print(DallasTemperature::toFahrenheit(tempC));
  }
}


void loop()
{    
    sensors.requestTemperatures();
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print("ARDUINO TEMP MONITOR");
    lcd.setCursor(0,1);
    lcd.print("Sensor1: ");
    displayTemperature(Probe01);
    lcd.setCursor(0,2);
    lcd.print("Sensor2: ");
    displayTemperature(Probe02);
    lcd.setCursor(0,3);
    lcd.print("Sensor3: ");
    displayTemperature(Probe03);
     
    File datafile = SD.open(filename, FILE_WRITE);
    if (datafile) {
         //while (datafile.available()) {
         //Serial.println(datafile.read());
         datafile.print(rtc.getDateStr());
         datafile.print(",");
         datafile.print(rtc.getTimeStr());
         datafile.print(",");
         datafile.print(sensors.getTempFByIndex(0));
         datafile.print(",");
         datafile.print(sensors.getTempFByIndex(1));
         datafile.print(",");
         datafile.println(sensors.getTempFByIndex(2));
         datafile.close();
    } else {
        Serial.println("SD Card Fail!");
    }
    delay(5000);
}
