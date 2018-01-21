/* Copyright (C) ********** - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Version 0.9 December, 2016
 */
 
#include <CurieIMU.h>
#include <CurieBLE.h>

BLEPeripheral blePeripheral; // BLE Peripheral Device (the board you're programming)


//Service declarations
BLEService batteryService("180F");                                // standard service UUID
BLEService tapService("6f31586c-265d-4008-8d3b-8d88bf51a94e");    // custom service UUID
BLEService crashService("386c8080-f7b1-49e6-93da-095a8c15420d");  // custom service UUID
BLEService buttonService("11d46303-4142-4e78-9aff-03d3a869b61a");  // custom service UUID

//Characteristic declartions and permissions
BLEUnsignedCharCharacteristic batteryLevelChar("2A19", BLERead | BLENotify);                                // standard attribute UUID
BLEIntCharacteristic tapStatusChar("2ce8c149-396f-4f01-8b18-ac934c9cb262", BLERead | BLENotify);   // custom attribute UUID
BLEIntCharacteristic crashStatusChar("d8b80845-76d5-446a-9f35-8130277c50a2", BLERead | BLENotify);          // custom attribute UUID
BLEIntCharacteristic buttonStatusChar("03991846-16fc-4b1b-bec4-23e0da77d2f2", BLERead | BLENotify);

//Global variables
int oldBatteryLevel = 0;    // last battery level reading from analog input
long previousMillis = 0;    // last time the battery level was checked, in ms
int tapStatus = 0;          // counter for number of double taps
int crashStatus = 0;         // crash status flag 
int buttonStatus = 0;
unsigned long lastInterrupt;

void setup() {
  Serial.begin(9600);    // initialize serial communication for debugging purposes
  pinMode(13, OUTPUT);   // initialize the LED on pin 13 to indicate when a central is connected
  
  blePeripheral.setLocalName("**********");                  //Set device name

  // add the service UUID's to the GAP
  blePeripheral.setAdvertisedServiceUuid(batteryService.uuid());  
  blePeripheral.setAdvertisedServiceUuid(tapService.uuid());
  blePeripheral.setAdvertisedServiceUuid(crashService.uuid());
  blePeripheral.setAdvertisedServiceUuid(buttonService.uuid());

  // battery level
  blePeripheral.addAttribute(batteryService);                     // add the BLE Battery service
  blePeripheral.addAttribute(batteryLevelChar);                   // add the battery level characteristic
  batteryLevelChar.setValue(oldBatteryLevel);                     // initial value for this characteristic

  // double tap detection
  blePeripheral.addAttribute(tapService);                         // add the double tap detection service
  blePeripheral.addAttribute(tapStatusChar);                      // add the tap status characteristic
  tapStatusChar.setValue(tapStatus);                              // intial value for this characteristic

  // crash detection
  blePeripheral.addAttribute(crashService);                        // add the crash detection service
  blePeripheral.addAttribute(crashStatusChar);                     // add the crash status characteristic
  tapStatusChar.setValue(crashStatus);                             // intial value for this characteristic
 // crash detection
  blePeripheral.addAttribute(buttonService);                        // add the crash detection service
  blePeripheral.addAttribute(buttonStatusChar);                     // add the crash status characteristic
  tapStatusChar.setValue(buttonStatus);                             // intial value for this characteristic
  
  
  //Serial.println("Bluetooth device active, waiting for connections...");
  blePeripheral.begin();

  attachInterrupt(2, interrupt1, CHANGE);
  attachInterrupt(3, interrupt2, CHANGE);
  attachInterrupt(4, interrupt3, CHANGE);
  
  CurieIMU.begin();
  CurieIMU.attachInterrupt(eventCallback);               // function to be called when interrupt is activated

  // set tap detection parameters
  CurieIMU.setAccelerometerRange(4);                     // increase accelerometer range to allow detection of stronger taps (< 4g)  
  CurieIMU.setDetectionThreshold(CURIE_IMU_TAP, 750);    // reduce threshold to allow detection of weaker taps (>= 750mg)  
  CurieIMU.interrupts(CURIE_IMU_DOUBLE_TAP);             // set event to trigger the interrupt. In this case a double tap

  //set crash detection parameters
  CurieIMU.setDetectionThreshold(CURIE_IMU_SHOCK, 15968.75); // 1g = 1000 mg
  CurieIMU.setDetectionDuration(CURIE_IMU_SHOCK, 75);        // duration in ms
  CurieIMU.interrupts(CURIE_IMU_SHOCK);                      // set event to trigger the interrupt. In this case a a shock of specified g's

  CurieIMU.setDetectionThreshold(CURIE_IMU_ZERO_MOTION, 7968.75); // 1g = 1000mg, threshold to determine what constitutes being motionless
  CurieIMU.setDetectionDuration(CURIE_IMU_ZERO_MOTION, 296.96);   // seconds for which Curie has to be motionless
  CurieIMU.interrupts(CURIE_IMU_ZERO_MOTION);                     // trigger interrupt if motionless as specified above

  //Serial.println("IMU initialization complete, waiting for events...");
  
}

void loop() {
  
  BLECentral central = blePeripheral.central();         // listen for BLE peripherals to connect

  
  if (central) {                                       // if a central is connected to peripheral:
    //Serial.print("Connected to central: ");    
    //Serial.println(central.address());                 // MAC address    
    digitalWrite(13, HIGH);                            // turn on the LED to indicate the connected status    
    
    while (central.connected()) {                      // since this is the main loop, all updation of characteristic variables will be updated here to get notifications
      tapStatusChar.setValue(tapStatus);               // update the tap service characteristic, triggers BLENotify
      crashStatusChar.setValue(crashStatus);           // update the crash service characteristic, triggers BLENotify
      buttonStatusChar.setValue(buttonStatus);
      long currentMillis = millis();                   // get the time passed since the execution began in ms      
      if (currentMillis - previousMillis >= 200) {     // if more than 200 ms passed, update battery
        previousMillis = currentMillis;
        updateBatteryLevel();
      }
    }    
    
    digitalWrite(13, LOW);                             // when the central disconnects, turn off the LED:
    //Serial.print("Disconnected from central: ");
    Serial.println(central.address());      
  }
}

void updateBatteryLevel() {

  int battery = analogRead(A3);
  int batteryLevel = map(battery, 0, 1023, 0, 100);
  
  

  if (batteryLevel != oldBatteryLevel) {      // if the battery level has changed
    Serial.print("Battery Level % is now: "); 
    Serial.println(batteryLevel);
    batteryLevelChar.setValue(batteryLevel);  // and update the battery level characteristic, triggers BLENotify
    oldBatteryLevel = batteryLevel;           // save the level for next comparison
  }
}

static void eventCallback()
{
   if (CurieIMU.getInterruptStatus(CURIE_IMU_DOUBLE_TAP)) {  // if a double tap is detected in any axis, the tap service characteristic is incremented
    tapStatus = tapStatus + 1;          
  }
  if (CurieIMU.getInterruptStatus(CURIE_IMU_SHOCK)) {        // if a crash is detected in any axis, the crash service characteristic is incremented
    crashStatus = crashStatus + 1;          
  }
  //if (CurieIMU.getInterruptStatus(CURIE_IMU_ZERO_MOTION)) {  // if motionless, switch of status LED and stop the BLE connection
    //digitalWrite(13, LOW);
    //blePeripheral.end();
    
  //}
}
void interrupt1() {
  if(millis() - lastInterrupt > 300) // we set a 10ms no-interrupts window
    {    
    Serial.write("one \n");
    buttonStatus = buttonStatus + 1;
    lastInterrupt = millis();
    }
}

void interrupt2() {
  if(millis() - lastInterrupt > 300) // we set a 10ms no-interrupts window
    {    
    Serial.write("two \n");
    buttonStatus = buttonStatus + 2;
    lastInterrupt = millis();
    }
}

void interrupt3() {
  if(millis() - lastInterrupt > 300) // we set a 10ms no-interrupts window
    {    
    Serial.write("three \n");
    buttonStatus = buttonStatus + 3;
    lastInterrupt = millis();
    }
}
