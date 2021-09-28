#include <TinyGPS++.h>
#include <SPI.h>
#include <LoRa.h>

#define LED1 5                // ESP32 GPIO connected to LED1
#define LED2 0                // ESP32 GPIO connected to LED2
#define LED_LORA_RX LED1      // Led that notifies LoRa RX
#define LED_LORA_TX LED2      // Led that notifies LoRa TX
#define CS_LORA 5             // LoRa radio chip select
#define RST_LORA 14           // LoRa radio reset
#define IRQ_LORA 13           // LoRa Hardware interrupt pin
#define LORA_SF  7            // LoRa Spreading Factor
#define MOTOR1   27           // Servo Motor steering pin
#define PWM_FREQ    50        //Pwm frequency for servo motor
#define PWM_CHANNEL 0         //PWM channel for single servo motor
#define PWM_RES     8         //PWM resolution
#define PWM_MOVE    13        // PWM constant move
#define BATTERY_PIN 33        //PIN to read battery value from
#define PERIOD_TX_LORA 1000   // Period between Lora Transmissions
#define PERIOD_ERASE_BUFF 10000 // Period between the erase of terminal msgs
#define PERIOD_TX_LORA 1000   // Period between Lora Transmissions
#define LORA_FREQUENCY 915E6  // Frequency used by the LoRa module
#define GPS_BAUD  9600        // GPS baud rate
#define GPS_RX    16
#define GPS_TX    17
#define TIME_PARAM_INPUT_1 "hours"  // Params used to parse the html form submit
#define TIME_PARAM_INPUT_2 "min"    // Params used to parse the html form submit
#define TIME_PARAM_INPUT_3 "sec"    // Params used to parse the html form submit
#define GPS_MESSAGE_ID 0
#define TIMER_MESSAGE_ID 1

