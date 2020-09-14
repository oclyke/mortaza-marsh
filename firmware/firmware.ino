#include <ArduinoBLE.h>
#include "Wire.h"

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service
BLEService accService("19B10100-E8F2-537E-4F6C-D104768A1214"); // Accelerometer Service
BLEService micService("19B10200-E8F2-537E-4F6C-D104768A1214"); // Microphone Service
BLEService camService("19B10300-E8F2-537E-4F6C-D104768A1214"); // Camera Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central 
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

// Onboard sensor characteristics - custom 128-bit UUID, readable by central (notify flag means writeValue will automaticaly push to central devices)
BLEByteCharacteristic magnitudeCharacteristic("19B10101-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEByteCharacteristic frequencyCharacteristic("19B10201-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEByteCharacteristic autoexposureCharacteristic("19B10301-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);

const int ledPin = LED_BUILTIN; // pin to use for the LED

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Artemis");
  BLE.setAdvertisedService(ledService);

  // add the characteristics to the services
  ledService.addCharacteristic(switchCharacteristic);
  accService.addCharacteristic(magnitudeCharacteristic);
  micService.addCharacteristic(frequencyCharacteristic);
  camService.addCharacteristic(autoexposureCharacteristic);

  // add service
  BLE.addService(ledService);
  BLE.addService(accService);
  BLE.addService(micService);
  BLE.addService(camService);

  // set the initial value for the characeristics:
  switchCharacteristic.writeValue(0);
  magnitudeCharacteristic.writeValue(1);
  frequencyCharacteristic.writeValue(2);
  autoexposureCharacteristic.writeValue(3);

  // start advertising
  BLE.advertise();

  Wire.begin();

  Serial.println("Artemis Dev Kit BLE Sensor Updater");
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }


      static uint32_t updateAcc = 0;
      if(millis() >= updateAcc){
        updateAcc = millis() + 1000;      
        magnitudeCharacteristic.writeValue(random(0,256));
//        Serial.print("Updated acc magnitude: ");
//        Serial.println(magnitudeCharacteristic.value());
      }


      static uint32_t updateMic = 0;
      if(millis() >= updateMic){
        updateMic = millis() + 1500;
        frequencyCharacteristic.writeValue(random(0,256));
//        Serial.print("Updated mic frequency: ");
//        Serial.println(frequencyCharacteristic.value());
      }


      static uint32_t updateCam = 0;
      if(millis() >= updateCam){
        updateCam = millis() + 2000;
        autoexposureCharacteristic.writeValue(random(0,256));
//        Serial.print("Updated cam autoexposure: ");
//        Serial.println(autoexposureCharacteristic.value());
      }
      
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
