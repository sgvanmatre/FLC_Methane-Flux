#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <MKRNB.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "ADS1X15.h"
#include <SensirionI2cSfmSf06.h>
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


// -----------------------TCA9548A DEF-----------------------------
#define TCA1_Address 0x70 // TCA9548A Encoders address
bool even = true;

void selectI2CChannels(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCA1_Address);
  Wire.write(1 << i);
  Wire.endTransmission();
}

// -----------------------BME DEF-----------------------------
#define BME_B_ADDRESS 0x77 
Adafruit_BME680 B_bme;

// -----------------------ADC DEF-----------------------------
ADS1115 B_ADS(0x49);

// ----------------------- Sim Card ---------------------------

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

  // ----------------------- SETUP -----------------------------
void setup() {
    //Initialize serial
  Serial.begin(115200);
  delay(1500);
    Wire.begin();

  // ----------------------- TCA9548A -----------------------------
  selectI2CChannels(0);

  // ----------------------- BME -----------------------------
  if(!B_bme.begin(BME_B_ADDRESS)){
    Serial.println("Could not find valid BME Main Sensor...");
    }
   Serial.println("BME Started");
    
  // Set up oversampling and filter initialization bme
  B_bme.setTemperatureOversampling(BME680_OS_8X);
  B_bme.setHumidityOversampling(BME680_OS_8X);
  B_bme.setPressureOversampling(BME680_OS_NONE);
  B_bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  B_bme.setGasHeater(320, 150); // 320*C for 150 ms

  // ----------------------- ADC -----------------------------
  B_ADS.begin();
  B_ADS.setGain(0);

// ----------------------- GMS Setup -----------------------------
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  
  // Wait for port to open
  start_and_connect_modem();

  // Init RTC 
  rtc.begin();
  //synchronizeRTC();

  Serial.println("End of startup"); 
}

// ----------------------- LOOP -----------------------------
void loop() {
  unsigned long currentMillis = millis(); 

// ----------------------- JSON File -----------------------------
 StaticJsonDocument<512> doc;

// ----------------------- BME Data -----------------------------
if(even){
  for (uint8_t channel = 0; channel < 6; channel++) {
    selectI2CChannels(channel);
    delay(100);
    
    B_bme.performReading();
    delay(10);

    doc[String("TEMP_")+ channel] = B_bme.temperature;
    doc[String("Humidity_")+ channel] = B_bme.humidity;
    doc[String("Pressure_")+ channel] = B_bme.pressure;
    doc[String("Gas_")+ channel] = B_bme.gas_resistance / 1000.0;
 }
  even= false;
 }
 // ----------------------- MOX Data -----------------------------
 else{
  for (uint8_t channel = 0; channel < 5; channel++) {
    selectI2CChannels(channel);
    delay(100);
    
    int16_t B_TGS00 = B_ADS.readADC(3);
    delay(10);
    int16_t B_TGS02 = B_ADS.readADC(1);
    delay(10);
    int16_t B_TGS11 = B_ADS.readADC(2);
    delay(10);

    doc[String("TGS00_") + channel] = B_TGS00;
    doc[String("TGS02_") + channel] = B_TGS02;
    doc[String("TGS11_") + channel] = B_TGS11;
  }
  even = true;
 }

  // ----------------------- Send data to SoraCOM ------------------
  // Make sure the device is still connected to CatM network
  if (nbAccess.isAccessAlive()) {
    Serial.println("Sending..."); 
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer); // print to client
    post_data(jsonBuffer); 
  } else {
    Serial.println("Modem disconnected, reconnecting");
    connected = false;
    //connect_modem();
  }

  delay(20000); 
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
