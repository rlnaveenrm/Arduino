#include <CurieBLE.h>
#include <CurieIMU.h>
#include <Power.h>

const int chatPin = 2;
const int assistPin = 3;
BLEPeripheral blePeripheral;       // BLE Peripheral Device (the board you're programming)
BLEService batteryService("180F"); // BLE Battery Service
BLEService buttonService("19B10010-E8F2-537E-4F6C-D104768A1214");


// BLE Battery Level Characteristic"
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",  // standard 16-bit characteristic UUID
    BLERead | BLENotify);     // remote clients will be able to
// get notifications if this characteristic changes
BLEUnsignedCharCharacteristic assistServiceChar("19B10011-E8F2-537E-4F6C-D104768A1214",
  BLERead | BLENotify);
BLEUnsignedCharCharacteristic chatServiceChar("19B10012-E8F2-537E-4F6C-D104768A1214",
  BLERead | BLENotify);

int oldBatteryLevel = 0;  // last battery level reading from analog input
long previousMillis = 0;  // last time the battery level was checked, in ms

void setup() {
  Serial.begin(9600);    // initialize serial communication
  pinMode(13, OUTPUT);   // initialize the LED on pin 13 to indicate when a central is connected
  pinMode(assistPin, INPUT);
  pinMode(chatPin, INPUT);

  /* Set a local name for the BLE device
     This name will appear in advertising packets
     and can be used by remote devices to identify this BLE device
     The name can be changed but maybe be truncated based on space left in advertisement packet */
  blePeripheral.setLocalName("BatteryMonitorSketch");
  blePeripheral.setAdvertisedServiceUuid(batteryService.uuid());  // add the service UUID
  blePeripheral.addAttribute(batteryService);   // Add the BLE Battery service
  blePeripheral.addAttribute(batteryLevelChar); // add the battery level characteristic
  batteryLevelChar.setValue(oldBatteryLevel);   // initial value for this characteristic

  
  blePeripheral.setAdvertisedServiceUuid(buttonService.uuid());
  blePeripheral.addAttribute(buttonService);
  blePeripheral.addAttribute(assistServiceChar);
  blePeripheral.addAttribute(chatServiceChar);

  assistServiceChar.setValue(0);
  chatServiceChar.setValue(0);
  
  /* Now activate the BLE device.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */
  blePeripheral.begin();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(13, HIGH);

    // check the battery level every 200ms
    // as long as the central is still connected:
    while (central.connected()) {
      // poll peripheral
      blePeripheral.poll();

  // read the current button pin state
      char assistValue = digitalRead(assistPin);
      char chatValue = digitalRead(chatPin);
  // has the value changed since the last read
      boolean assistChanged = (assistServiceChar.value() != assistValue);
      
      boolean chatChanged = (chatServiceChar.value() != chatValue);
      if (assistChanged) {
    // button state changed, update characteristics
         assistServiceChar.setValue(assistValue);
         
      }
      if (chatChanged) {
    // button state changed, update characteristics
         chatServiceChar.setValue(chatValue);
      }
      long currentMillis = millis();
      // if 200ms have passed, check the battery level:
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;
        updateBatteryLevel();
      }
      
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(13, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void updateBatteryLevel() {
  /* Read the current voltage level on the A0 analog input pin.
     This is used here to simulate the charge level of a battery.
  */
  int battery = analogRead(A0);
  int batteryLevel = map(battery, 0, 1023, 0, 100);

  if (batteryLevel != oldBatteryLevel) {      // if the battery level has changed
    Serial.print("Battery Level % is now: "); // print it
    Serial.println(batteryLevel);
    batteryLevelChar.setValue(batteryLevel);  // and update the battery level characteristic
    oldBatteryLevel = batteryLevel;           // save the level for next comparison
  }
}
