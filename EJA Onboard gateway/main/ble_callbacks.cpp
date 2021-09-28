#include "ble_callbacks.hpp"

extern uint32_t timer_end_ts_local;
extern bool send_updated_timer;



void NewTimeCharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic){
    try{
        timer_end_ts_local = millis() +  strtoul(pCharacteristic->getValue().c_str(), NULL, 0);
        send_updated_timer = true;
    }catch(const std::exception &e){
        Serial.println(e.what());
    }
}


