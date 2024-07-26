#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <MKRNB.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "ADS1X15.h"
#include <SensirionI2cSfmSf06.h>
#include <SPI.h>
#include <SD.h>
#include <RTCZero.h>

// macro definitions -------------------------------------
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

static char errorMessage[64];
static int16_t error;

SensirionI2cSfmSf06 sensor;

// -----------------------BME DEF-----------------------------
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_A_ADDRESS 0x76
#define BME_B_ADDRESS 0x77
Adafruit_BME680 A_bme; 
Adafruit_BME680 B_bme;

// -----------------------ADC DEF-----------------------------
ADS1115 A_ADS(0x48);
ADS1115 B_ADS(0x49);

// ----------------------- SD Card ---------------------------
const int chipSelect = 4;  // SD card CS pin

// Debug options
#define PRINT_AT        true   // Show or hide AT command output from the modem

// PIN Number
const char PINNUMBER[]     = "";
char apn[] = "soracom.io";
char user[] = "sora";
char pass[] = "sora";

// Server details
const char server[] = "unified.soracom.io";
const int  port = 80; 


NBClient client;
GPRS gprs;
NB nbAccess(PRINT_AT);
HttpClient http(client, server, port);

// Init on-board RTC
RTCZero rtc;

// connection state
bool connected = false;

// Publish interval
long previousMillis = 0; 
long interval = 20000; // milliseconds

void setup() {
    //Initialize serial
  Serial.begin(115200);
  delay(5000);

  // ------------ SD Card -----------------
  Serial.println("Initializing SD card...");

  // See if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed, or not present");
    delay(500);
    Serial.println("Data will not be written to SD Card");
  }
  else{
    Serial.println("SD Card initialized.");
  }

  
  // ----------------------- BME -----------------------------
  if(!A_bme.begin(BME_A_ADDRESS)){
    Serial.println("Could not find valid BME Ambiant Sensor..."); 
    }
  if(!B_bme.begin(BME_B_ADDRESS)){
    Serial.println("Could not find valid BME Main Sensor...");
    }
   Serial.println("BME Started");
    
  // Set up oversampling and filter initialization ^bme1 and bme2^ 
  A_bme.setTemperatureOversampling(BME680_OS_8X);
  A_bme.setHumidityOversampling(BME680_OS_8X);
  A_bme.setPressureOversampling(BME680_OS_NONE);
  A_bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  A_bme.setGasHeater(320, 150); // 320*C for 150 ms

  B_bme.setTemperatureOversampling(BME680_OS_8X);
  B_bme.setHumidityOversampling(BME680_OS_8X);
  B_bme.setPressureOversampling(BME680_OS_NONE);
  B_bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  B_bme.setGasHeater(320, 150); // 320*C for 150 ms

  // ----------------------- ADC -----------------------------
  A_ADS.begin();
  A_ADS.setGain(0);
  
  B_ADS.begin();
  B_ADS.setGain(0);

// ----------------------- GMS Setup -----------------------------
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  
  // Wait for port to open
  start_and_connect_modem();

  //    ----------------------- FML -----------------------------
    sensor.begin(Wire, SFM4300_I2C_ADDR_2A);
    Serial.println("Connection to Flow");

    sensor.stopContinuousMeasurement();
    delay(100);
    uint32_t productIdentifier = 0;
    uint8_t serialNumber[8] = {0};
    error = sensor.readProductIdentifier(productIdentifier, serialNumber, 8);
    Serial.println(error);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readProductIdentifier(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    Serial.print("productIdentifier: ");
    Serial.print(productIdentifier);
    Serial.print("\t");
    Serial.print("serialNumber: ");
    print_byte_array(serialNumber, 8);
    Serial.println();
    error = sensor.startO2ContinuousMeasurement();
    if (error != NO_ERROR) {
        Serial.print(
            "Error trying to execute startO2ContinuousMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }

    Serial.println("Flow Started");

  Serial.println("End of startup"); 
  
}

void loop() {
  unsigned long currentMillis = millis(); 

    //----------------------- BME READING -----------------------------
  A_bme.performReading();
  delay(10);
  B_bme.performReading();
  delay(10);
  
  // ----------------------- ADC READING -----------------------------
  int16_t A_TGS00 = A_ADS.readADC(0);
  delay(10);
  int16_t A_TGS02 = A_ADS.readADC(1);
  delay(10);
  int16_t A_TGS11 = A_ADS.readADC(2);
  delay(10);

  int16_t B_TGS00 = B_ADS.readADC(3);
  delay(10);
  int16_t B_TGS02 = B_ADS.readADC(1);
  delay(10);
  int16_t B_TGS11 = B_ADS.readADC(2);
  delay(10);

  // -------------------- Flow Meter READING --------------------------
    float aFlow = 0.0;
    float aTemperature = 0.0;
    uint16_t aStatusWord = 0u;
    delay(100);
    error = sensor.readMeasurementData(aFlow, aTemperature, aStatusWord);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readMeasurementData(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
  
  // ----------------------- Send Data to SD card -----------------
  delay(100);
  logDataToSD(currentMillis, aFlow, aTemperature, A_bme.temperature, A_bme.humidity, A_bme.pressure, A_bme.gas_resistance / 1000.0, A_TGS00, A_TGS02, A_TGS11, B_bme.temperature, B_bme.humidity, B_bme.pressure, B_bme.gas_resistance / 1000.0, B_TGS00, B_TGS02, B_TGS11);
  delay(100);

  // ----------------------- Send data to SoraCOM ------------------
  // Make sure the device is still connected to CatM network
  if (nbAccess.isAccessAlive()) {
    Serial.println("Sending..."); 
      // Construct the JSON data to send
      StaticJsonDocument<200> doc;
      doc["aFlow"] = aFlow;
      doc["aTemp"] = aTemperature;
      doc["A_TEMP"] = A_bme.temperature;
      doc["A_Humidity"] = A_bme.humidity;
      doc["A_Pressure"] = A_bme.pressure;
      doc["A_Gas"] = A_bme.gas_resistance / 1000.0;
      doc["A_TGS00"] = A_TGS00;
      doc["A_TGS02"] = A_TGS02;
      doc["A_TGS11"] = A_TGS11;
      doc["B_TEMP"] = B_bme.temperature;
      doc["B_Humidity"] = B_bme.humidity;
      doc["B_Pressure"] = B_bme.pressure;
      doc["B_Gas"] = B_bme.gas_resistance / 1000.0;
      doc["B_TGS00"] = B_TGS00;
      doc["B_TGS02"] = B_TGS02;
      doc["B_TGS11"] = B_TGS11;
      char jsonBuffer[512];
      serializeJson(doc, jsonBuffer); // print to client
      post_data(jsonBuffer); 

  } else {
    Serial.println("Modem disconnected, reconnecting");
    connected = false;
    //connect_modem();
  }

  delay(5000); 
}

// ------------------------ SD Card Writing ----------------------------
void logDataToSD(unsigned long currentMillis, float aFlow, float aTemperature, float A_TEMP, float A_Humidity, float A_Pressure, float A_Gas, int16_t A_TGS00, int16_t A_TGS02, int16_t A_TGS11, float B_TEMP, float B_Humidity, float B_Pressure, float B_Gas, int16_t B_TGS00, int16_t B_TGS02, int16_t B_TGS11) {
  // boolean for writing column headers
  bool fileExists = SD.exists("datalog.csv");
  // Open the file for appending
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  // if file does not exist yet (fileExists == FALSE), write preset column headers
  if (!fileExists) {
    dataFile.println("Year-Month-Day Hour/Min/Sec,Flux Chamber,aFlow,aTemperature,A_Temperature,A_Humidity,A_Pressure,A_Gas,A_TGS00,A_TGS02,A_TGS11,B_Temperature,B_Humidity,B_Pressure,B_Gas,B_TGS00,B_TGS02,B_TGS11");
    Serial.println("Header written to SD card");
  } 

  // If the file is open, write to it
  if (dataFile) {
    // Read current time from RTC
    int currentSeconds = rtc.getSeconds();
    int currentMinutes = rtc.getMinutes();
    int currentHours = rtc.getHours();
    int currentDay = rtc.getDay();
    int currentMonth = rtc.getMonth();
    int currentYear = rtc.getYear();
    // Write the data
    dataFile.print(String(currentYear) + "-" + String(currentMonth) + "-" + String(currentDay) + " " + String(currentHours) + ":" +
                     String(currentMinutes) + ":" + String(currentSeconds));
    dataFile.print(",");
    dataFile.print("1"); // Need to change to whichever flux chamber you are running
    dataFile.print(",");
    dataFile.print(aFlow);
    dataFile.print(",");
    dataFile.print(aTemperature);
    dataFile.print(",");
    dataFile.print(A_TEMP);
    dataFile.print(",");
    dataFile.print(A_Humidity);
    dataFile.print(",");
    dataFile.print(A_Pressure);
    dataFile.print(",");
    dataFile.print(A_Gas);
    dataFile.print(",");
    dataFile.print(A_TGS00);
    dataFile.print(",");
    dataFile.print(A_TGS02);
    dataFile.print(",");
    dataFile.print(A_TGS11);
    dataFile.print(",");
    dataFile.print(B_TEMP);
    dataFile.print(",");
    dataFile.print(B_Humidity);
    dataFile.print(",");
    dataFile.print(B_Pressure);
    dataFile.print(",");
    dataFile.print(B_Gas);
    dataFile.print(",");
    dataFile.print(B_TGS00);
    dataFile.print(",");
    dataFile.print(B_TGS02);
    dataFile.print(",");
    dataFile.print(B_TGS11);
    dataFile.println();
    dataFile.close();
    Serial.println("Data written to SD card");
  } else {
    Serial.println("Error opening datalog.csv");
  }
}

// --------------------------- Cellular ---------------------------
void post_data(String postData) {
  Serial.println("making POST request");
  String contentType = "application/json";

  http.post("/", contentType, postData);

  // read the status code and body of the response
  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

void start_and_connect_modem(){
  // Send Poweron pulse
  digitalWrite(SARA_PWR_ON, HIGH);
  delay(300);
  digitalWrite(SARA_PWR_ON, LOW);
  delay(1000);
  // Turn on the modem
  if (Serial) Serial.print(">>> Turning on modem...");
  if (MODEM.begin()) {
    if (Serial) Serial.println(" done!");
  } else {
    if (Serial) Serial.println(" error, could not turn on modem! Try power-cycling.");
    return;
  }

  while (!MODEM.noop());
  MODEM.sendf("ATI9");
  MODEM.waitForResponse(2000);
  
  // Run AT commands to reset the modem to global default settings
  Serial.print(">>> Resetting modem to default settings...");
  MODEM.sendf("AT+CFUN=0");
  MODEM.waitForResponse(6000);
  MODEM.sendf("AT+UMNOPROF=2");
  MODEM.waitForResponse(6000);
  MODEM.sendf("AT+CFUN=15");
  MODEM.waitForResponse(6000);
  MODEM.sendf("AT+CFUN=0");
  MODEM.waitForResponse(6000);
  MODEM.sendf("AT+UMNOPROF=0");
  MODEM.waitForResponse(6000);
  MODEM.sendf("AT+CFUN=15");
  MODEM.waitForResponse(6000);
  MODEM.sendf("AT+URAT=7");
  MODEM.waitForResponse(6000);
  MODEM.sendf("AT+UBANDMASK?");
  MODEM.waitForResponse(6000);
  Serial.println(" done!");
  
  delay(2000);
  // attempt to connect to GSM and GPRS:
  Serial.print("Attempting to connect to GSM and GPRS");
   
  connect_modem();
  
}

void connect_modem(){
  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  MODEM.sendf("AT+CMEE=2");
  MODEM.waitForResponse(6000);
  while (!connected) {
    if ((nbAccess.begin(PINNUMBER, apn, user, pass) == NB_READY) &&
        (gprs.attachGPRS() == GPRS_READY)) {
      Serial.println("Connected Now");
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("You're connected to the network");  
}

void print_byte_array(uint8_t* array, uint16_t len) {
    uint16_t i = 0;
    Serial.print("0x");
    for (; i < len; i++) {
        Serial.print(array[i], HEX);
    }
}
