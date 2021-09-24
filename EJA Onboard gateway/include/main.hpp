#include <Arduino.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <LoRa.h>

#include <ArduinoJson.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


#define LED1 22                 // ESP32 GPIO connected to LED1
#define LED2 5                  // ESP32 GPIO connected to LED2
#define LED3 16                 // ESP32 GPIO connected to LED3
#define LED4 17                 // ESP32 GPIO connected to LED4
#define LED_LORA_RX LED1        // Led that notifies LoRa RX
#define LED_LORA_TX LED2        // Led that notifies LoRa TX
#define CS_LORA 2               // LoRa radio chip select
#define RST_LORA 15             // LoRa radio reset
#define IRQ_LORA 13             // LoRa Hardware interrupt pin
#define PERIOD_TX_LORA 1000     // Period between Lora Transmissions
#define LORA_FREQUENCY 915E6    // Frequency used by the LoRa module
#define TIME_PARAM_INPUT_1 "hours"  // Params used to parse the html form submit
#define TIME_PARAM_INPUT_2 "min"    // Params used to parse the html form submit
#define TIME_PARAM_INPUT_3 "sec"    // Params used to parse the html form submit

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define DEVICE_NAME "ESP32 LoRa Gateway"