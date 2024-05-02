/*
This is a test to see if the sensors are responding to one another. 
I'm so hungry they got me doing slave work over here. 
 */
#include <Arduino.h>
#include <SensirionI2cSfmSf06.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "ADS1X15.h"

// macro definitions -------------------------------------
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

SensirionI2cSfmSf06 sensor;

static char errorMessage[64];
static int16_t error;

void print_byte_array(uint8_t* array, uint16_t len) {
    uint16_t i = 0;
    Serial.print("0x");
    for (; i < len; i++) {
        Serial.print(array[i], HEX);
    }
}

// -----------------------BME DEF-----------------------------
#define BME_A_ADDRESS 0x76
#define BME_B_ADDRESS 0x77
Adafruit_BME680 A_bme;
Adafruit_BME680 B_bme;


// -----------------------ADC DEF-----------------------------
ADS1115 A_ADS(0x48);
ADS1115 B_ADS(0x49);

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    Wire.begin();
     // ----------------------------------------- FML -----------------------------------------
    sensor.begin(Wire, SFM4300_I2C_ADDR_2A);

    sensor.stopContinuousMeasurement();
    delay(100);
    uint32_t productIdentifier = 0;
    uint8_t serialNumber[8] = {0};
    error = sensor.readProductIdentifier(productIdentifier, serialNumber, 8);
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
  // ----------------------------------------- BME -----------------------------------------
  if (!A_bme.begin(BME_A_ADDRESS)) {
   Serial.println("Could not find a valid A_BME680 sensor, check wiring!");
   while(1);
  }
  
  if (!B_bme.begin(BME_B_ADDRESS)) {
   Serial.println("Could not find a valid B_BME680 sensor, check wiring!");
   while(1);
  }

  // Set up oversampling and filter initialization
  A_bme.setTemperatureOversampling(BME680_OS_8X);
  A_bme.setHumidityOversampling(BME680_OS_4X);
  A_bme.setPressureOversampling(BME680_OS_2X);
  A_bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  A_bme.setGasHeater(320, 150); // 320*C for 150 ms

  B_bme.setTemperatureOversampling(BME680_OS_8X);
  B_bme.setHumidityOversampling(BME680_OS_4X);
  B_bme.setPressureOversampling(BME680_OS_2X);
  B_bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  B_bme.setGasHeater(320, 150); // 320*C for 150 ms

  
  // ----------------------------------------- ADS -----------------------------------------
  A_ADS.begin();
  A_ADS.setGain(0);

  B_ADS.begin();
  B_ADS.setGain(0);

 Serial.println("Flow, A_temp, A_Hum, A_TGS2600, A_TGS2602, A_TGS2611, B_temp, B_hum, B_TGS2600, B_TGS2602, B_TGS2611");
}

void loop() {

    
    A_bme.performReading();
    delay(25);
    B_bme.performReading();
    delay(25);
    uint16_t A_TGS00 = A_ADS.readADC(0);  //2600 
    uint16_t A_TGS02 = A_ADS.readADC(1);  //2602
    uint16_t A_TGS11 = A_ADS.readADC(2);  //2611

    delay(25);
    uint16_t B_TGS00 = B_ADS.readADC(3);  //2600 
    uint16_t B_TGS02 = B_ADS.readADC(1);  //2602
    uint16_t B_TGS11 = B_ADS.readADC(2);  //2611
    delay(200);

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
    

//    Serial.print("aFlow: ");
    Serial.print(aFlow);
    Serial.print("\t");
//    Serial.print("aTemperature: ");
//    Serial.print(aTemperature);

//    Serial.print("A_bTemp: ");
//    Serial.print(A_bme.temperature);
//    Serial.print("\t");
////    Serial.print("A_bHumidyt: ");
//    Serial.print(A_bme.humidity);
//
//    Serial.print("\t");
////    Serial.print("A_TGS00: ");
//    Serial.print(A_TGS00);
//    Serial.print("\t");
////    Serial.print("A_TGS02: ");
//    Serial.print(A_TGS02);
//    Serial.print("\t");
////    Serial.print("A_TGS11: ");
//    Serial.print(A_TGS11);
    
    Serial.print("\t");
//    Serial.print("B_bTemp: ");
    Serial.print(B_bme.temperature);
    Serial.print("\t");
    Serial.print(B_bme.pressure / 100.0);
    Serial.print("\t");
//    Serial.print("B_bHumidyt: ");
    Serial.print(B_bme.humidity);
    
    Serial.print("\t");
//    Serial.print("B_TGS00: ");
    Serial.print(B_TGS00);
    Serial.print("\t");
//    Serial.print("B_TGS02: ");
    Serial.print(B_TGS02);
    Serial.print("\t");
//    Serial.print("B_TGS11: ");
    Serial.print(B_TGS11);
    Serial.println("");
    
    delay(1000);

}
