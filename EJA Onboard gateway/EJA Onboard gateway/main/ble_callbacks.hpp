#ifndef BLE_CB_H
#define BLE_CB_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <exception>

#define GT_CHARACTERISTIC_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define NODE_CHARACTERISTIC_UUID "beb5483f-36e1-4688-b7f5-ea07361b26a8"

#define TIME_CHARACTERISTIC_UUID "beb54840-36e1-4688-b7f5-ea07361b26a8"

#define NEW_TIME_CHARACTERISTIC_UUID "beb54841-36e1-4688-b7f5-ea07361b26a8"

#define GPS_CHARACTERISTIC_UUID "beb54842-36e1-4688-b7f5-ea07361b26a8"

#define BATTERY_CHARACTERISTIC_UUID "beb54843-36e1-4688-b7f5-ea07361b26a8"



class GatewayTimerCharacteristicCallbacks: public BLECharacteristicCallbacks{
    
};

class NodeTimerCharacteristicCallbacks: public BLECharacteristicCallbacks{
    
};

class SharedTimerCharacteristicCallbacks: public BLECharacteristicCallbacks{
    
};

class GPSCharacteristicCallbacks: public BLECharacteristicCallbacks{
    
};

class BatteryCharacteristicCallbacks: public BLECharacteristicCallbacks{
    
};

class NewTimeCharacteristicCallbacks: public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic *pCharacteristic);
};

#endif