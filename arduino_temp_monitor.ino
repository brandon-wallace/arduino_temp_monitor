// ARDUINO TEMPERATURE MONITOR //////////////////////////

#include <DS3231.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>


Time now;
int count = 1;
int line_number = 1;
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
    Serial.end();
    Serial.begin(9600);

    // i2C LCD //////////////////////////////////////////
    Serial.println("Initializing i2c LCD");
    lcd.begin(20,4);
    
     // SD Card //////////////////////////////////////////
    Serial.println("Initializing SD card... ");
    if (!SD.begin(4)) {
        Serial.println("SD CARD NOT FOUND!");
        lcd.clear();
        lcd.setCursor(1,0); 
        lcd.print("ERROR:");
        lcd.setCursor(1,2);
        lcd.print("SD Card not found.");
        return;
    }
    Serial.println("SD Card initialized.");
    
    // DS3231 real time clock ///////////////////////////
    Serial.println("Initializing DS3231 real time clock");
    rtc.begin();
    // Uncomment the following three lines to set date and time.
    //rtc.setDOW(THURSDAY);     // Set the day of the week to THURSDAY.
    //rtc.setTime(3, 21, 30);     // Set time HH MM SS (24hr format).
    //rtc.setDate(19, 10, 2017);   // Set date DD MM YYYY.
    now = rtc.getTime();
    
    //String(String(now.year) + String(now.mon) + String(now.date) + ".csv").toCharArray(filename, 13);
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
  
    // DS18B20 One-Wire Sensors /////////////////////////
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
}

// Read temperature from sensors.
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

// Create a new file name once the clock hits midnight.
void createFilename(){
    // Add a leading zero to day and month digits less that 10.
    String date_var = "";
    int d = now.date + 1;
    if(d < 10) { date_var += '0'; }
    date_var += d;

    String month_var = "";
    int m = now.mon;
    if(m < 10) { month_var += '0'; }
    month_var += m;
    String(String(now.year) + String(month_var) + String(date_var) + ".csv").toCharArray(filename, 13);
    File datafile = SD.open(filename, FILE_WRITE);
            if (datafile) {
                datafile.print("Number");
                datafile.print(",");
                datafile.print("Date");
                datafile.print(",");
                datafile.print("Time");
                datafile.print(",");
                datafile.print("Sensor 1");
                datafile.print(",");
                datafile.print("Sensor 2");
                datafile.print(",");
                datafile.println("Sensor 3");
                datafile.close();
            }
}

void loop()
{    
    now = rtc.getTime();
    sensors.requestTemperatures();
    //lcd.clear();  // Caused screen flicker.
    lcd.setCursor(1,0); 
    lcd.print(now.year);
    if (now.mon < 10) { lcd.print("0"); }
    lcd.print(now.mon);
    if (now.date < 10) { lcd.print("0"); }
    lcd.print(now.date);
    lcd.print("  ");
    if (now.hour < 10) { lcd.print("0"); }
    lcd.print(now.hour);
    lcd.print(":");
    if (now.min < 10) { lcd.print("0"); }
    lcd.print(now.min);
    lcd.print(":");
    if (now.sec < 10) { lcd.print("0"); }
    lcd.print(now.sec);
    lcd.setCursor(1,1);
    lcd.print("Sensor1: ");
    displayTemperature(Probe01);
    lcd.setCursor(1,2);
    lcd.print("Sensor2: ");
    displayTemperature(Probe02);
    lcd.setCursor(1,3);
    lcd.print("Sensor3: ");
    displayTemperature(Probe03);
    
    String time_now = rtc.getTimeStr();
    String midnight = "00:00:00";
    if (time_now == midnight) {
        createFilename();
        count = 1;  
        line_number = 1;        
        File datafile = SD.open(filename, FILE_WRITE);
        // Write to SD card every 10 minutes.
        } else if (now.min % 10 == 0 && now.sec == 0) {
            File datafile = SD.open(filename, FILE_WRITE);
            if (datafile) {
                datafile.print(line_number);
                datafile.print(",");
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
                line_number++; 
           } else {
           Serial.println("SD Card Failure");
           lcd.clear();
           lcd.setCursor(1,0); 
           lcd.print("ERROR:");
           lcd.setCursor(1,2); 
           lcd.print("SD Card Failure");
        }
    }
    count++;
    delay(1000);
}
