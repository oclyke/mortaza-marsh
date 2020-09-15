#include <ArduinoBLE.h>
#include "Wire.h"
#include "lis2dh12_platform_arduino.h"

#define INITIAL_UPDATE_PERIOD (250) // ms

axis3bit16_t data_raw_acceleration;
axis1bit16_t data_raw_temperature;
float acceleration_mg[3];
float temperature_degC;

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service
BLEService accService("19B10100-E8F2-537E-4F6C-D104768A1214"); // Accelerometer Service
BLEService updateService("19B10400-E8F2-537E-4F6C-D104768A1214"); // Update Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central 
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

// Onboard sensor characteristics - custom 128-bit UUID, readable by central (notify flag means writeValue will automaticaly push to central devices)
BLEByteCharacteristic accXCharacteristic("19B10101-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEByteCharacteristic accYCharacteristic("19B10102-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEByteCharacteristic accZCharacteristic("19B10103-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEByteCharacteristic accTCharacteristic("19B10104-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEByteCharacteristic periodCharacteristic("19B10401-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

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
  accService.addCharacteristic(accXCharacteristic);
  accService.addCharacteristic(accYCharacteristic);
  accService.addCharacteristic(accZCharacteristic);
  accService.addCharacteristic(accTCharacteristic);
  updateService.addCharacteristic(periodCharacteristic);

  // add service
  BLE.addService(ledService);
  BLE.addService(accService);

  // set the initial value for the characeristics:
  switchCharacteristic.writeValue(0);
  accXCharacteristic.writeValue(0);
  accYCharacteristic.writeValue(0);
  accZCharacteristic.writeValue(0);
  accTCharacteristic.writeValue(0);
  periodCharacteristic.writeValue(INITIAL_UPDATE_PERIOD);

  // start advertising
  BLE.advertise();

  Wire.begin();
  Wire.setClock(100000);
  initAccel(); // reqd to begin measurements!

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
      
      /* led */
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }

      
      static uint32_t update_debounce = 0;
      if(millis() >= update_debounce){
        update_debounce = millis() + periodCharacteristic.value(); // use update period to debouce updates

        /* accelerometer */
        updateAccelerometer();
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}

void updateAccelerometer( void ){
  lis2dh12_reg_t reg;
  
  // Check if accelerometer is ready with new temperature data
  lis2dh12_temp_data_ready_get(&dev_ctx, &reg.byte);
  if (reg.byte)    {
    /* Read temperature data */
    lis2dh12_temperature_raw_get(&dev_ctx, data_raw_temperature.u8bit);
  
    /* Convert to celsius */
    temperature_degC = lis2dh12_from_lsb_hr_to_celsius(data_raw_temperature.i16bit);

    accTCharacteristic.writeValue(data_raw_temperature.i16bit/128);

//    Serial.println("updated acc temp val");
  }
  
  // Check if accelerometer is ready with new acceleration data
  lis2dh12_xl_data_ready_get(&dev_ctx, &reg.byte);
  if (reg.byte){
    /* Read acceleration data */
    lis2dh12_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);

    /* convert to mg */
    acceleration_mg[0] = lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[0]);
    acceleration_mg[1] = lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[1]);
    acceleration_mg[2] = lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[2]);

    accXCharacteristic.writeValue(data_raw_acceleration.i16bit[0]/256);
    accYCharacteristic.writeValue(data_raw_acceleration.i16bit[1]/256);
    accZCharacteristic.writeValue(data_raw_acceleration.i16bit[2]/256);

//    Serial.println("updated acc vals");
  }
}

uint32_t initAccel( void ){

    uint32_t retVal32 = 0;
    static uint8_t whoamI = 0;

    //
    // Apply accelerometer configuration
    do {
      Serial.println("starting accelerometer...");
      lis2dh12_device_id_get(&dev_ctx, &whoamI);
      delay(250);
    }while(whoamI != LIS2DH12_ID);

    lis2dh12_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
    lis2dh12_temperature_meas_set(&dev_ctx, LIS2DH12_TEMP_ENABLE);
    lis2dh12_data_rate_set(&dev_ctx, LIS2DH12_ODR_25Hz);
    lis2dh12_full_scale_set(&dev_ctx, LIS2DH12_2g);
    lis2dh12_temperature_meas_set(&dev_ctx, LIS2DH12_TEMP_ENABLE);
    lis2dh12_operating_mode_set(&dev_ctx, LIS2DH12_HR_12bit);

    return 0;
}
