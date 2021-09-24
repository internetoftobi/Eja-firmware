#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


class GatewayTimerCharacteristicCallbacks: public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic *pCharacteristic);
};


class NodeTimerCharacteristicCallbacks: public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic *pCharacteristic);
};