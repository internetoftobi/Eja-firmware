# Eja firmware Dependencies
This directory is for EJA LoRa device.
It was implemented using BLE (Bluetooth Low Energy) configuration on the ESP32.

You can flash both buoy and onboard gateway via ArduinoIDE .
Before you do that, you need to install few ESP32 libraries:
- Both buoy and onboard gateway
	- ArduinoJson
	- LoRa
	
- Buoy:
	- TinyGPS++

# Starting EJA Firmware

After flashing code to both buoy and onboard gateway, connecting hardware modules/cables and starting them, onboard gateway should automatically get data from node. After getting data from node gateway set this data in BLE characteristics. onboard gateway can be found and connected via created mobile app or you can use external android app for testing like f.e: *BLE Scanner*.
Gateway name is set for: **ESP32 LoRa Gateway**. You can change that name and other settings (UUID of value charcteristics/service) by changing 
```c
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define DEVICE_NAME "ESP32 LoRa Gateway"
```
in `main.hpp` and 
```c
#define GT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define NODE_CHARACTERISTIC_UUID "beb5483f-36e1-4688-b7f5-ea07361b26a8"
#define TIME_CHARACTERISTIC_UUID "beb54840-36e1-4688-b7f5-ea07361b26a8"
#define NEW_TIME_CHARACTERISTIC_UUID "beb54841-36e1-4688-b7f5-ea07361b26a8"
#define GPS_CHARACTERISTIC_UUID "beb54842-36e1-4688-b7f5-ea07361b26a8"
#define BATTERY_CHARACTERISTIC_UUID "beb54843-36e1-4688-b7f5-ea07361b26a8"
```
in `ble_callbacks.hpp` files.  
For debugging possible errors and listening to devices you can look in Serial monitor of onboard gateway and/or buoy. Every message get from LoRa etc is printed to Serial port. 


