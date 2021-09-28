#include "main.hpp"

BLECharacteristic *pCharacteristic;
BLECharacteristic *gtCharacteristic;
BLECharacteristic *nodeCharacteristic;
BLECharacteristic *timeCharacteristic;
BLECharacteristic *newTimeCharacteristic;
BLECharacteristic *gpsCharacteristic;
BLECharacteristic *btrCharacteristic;

bool lora_exist;


// Variables of the timer
int timer_end_hours = 0;
int timer_end_min = 0;
int timer_end_sec = 0;
uint32_t timer_end_ts_shared = 0;
unsigned long timer_end_ts_local = 0;
unsigned long timer_current = 0;
StaticJsonDocument<512> gateway_doc;
// Time (millis) counters for timed operations
unsigned long last_msg_rx = 0;
unsigned long lora_last_tx = 0;

bool send_updated_timer = false;
bool reseting_timer = false;

//prototype functions
bool runEvery(unsigned long interval, unsigned long *previousMillis);
//lora function prototype
void LoRa_rxMode();
void LoRa_txMode();
void LoRa_sendMessage(String message);
void onReceiveLora(int packetSize); 
void onTxDoneLoRa();
String get_remaining_time(unsigned long timer_end_millis);


void setup() {
  // Initialize serial communication (used for debugging)
  Serial.begin(115200);
  LoRa.setPins(CS_LORA, RST_LORA, IRQ_LORA);
  if(!LoRa.begin(LORA_FREQUENCY)){
    Serial.println("LoRa init failed. Check your connections.");
    lora_exist = false;
  }else{
    lora_exist = true;
	LoRa.setSpreadingFactor(LORA_SF);
    LoRa.onReceive(onReceiveLora);
    LoRa.onTxDone(onTxDoneLoRa);
    // Print LoRa initialization messages
    Serial.println("LoRa init succeeded.");
    Serial.println();
    Serial.println("LoRa Simple Gateway");
    Serial.println("Only receive messages from nodes");
    Serial.println("Tx: invertIQ enable");
    Serial.println("Rx: invertIQ disable");
    Serial.println();
  }
  //initialize BLE
  BLEDevice::init(DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  gpsCharacteristic = pService->createCharacteristic(
                                         GPS_CHARACTERISTIC_UUID, 
                                         BLECharacteristic::PROPERTY_READ
                                       );
  gtCharacteristic = pService->createCharacteristic(
                                         GT_CHARACTERISTIC_UUID, 
                                         BLECharacteristic::PROPERTY_READ
                                       );
  nodeCharacteristic = pService->createCharacteristic(
                                         NODE_CHARACTERISTIC_UUID, 
                                         BLECharacteristic::PROPERTY_READ
                                       );
  btrCharacteristic = pService->createCharacteristic(
                                         BATTERY_CHARACTERISTIC_UUID, 
                                         BLECharacteristic::PROPERTY_READ
                                       );
  timeCharacteristic = pService->createCharacteristic(
                                         TIME_CHARACTERISTIC_UUID, 
                                         BLECharacteristic::PROPERTY_READ
                                       );
  newTimeCharacteristic = pService->createCharacteristic(
                                         NEW_TIME_CHARACTERISTIC_UUID, 
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  gpsCharacteristic->setValue("26.39164;54.34528"); //lat;lng format
  gpsCharacteristic->setCallbacks(new GPSCharacteristicCallbacks());
  gtCharacteristic->setValue("0");
  gtCharacteristic->setCallbacks(new GatewayTimerCharacteristicCallbacks());
  nodeCharacteristic->setValue("0");
  nodeCharacteristic->setCallbacks(new NodeTimerCharacteristicCallbacks());
  timeCharacteristic->setValue("0");
  timeCharacteristic->setCallbacks(new SharedTimerCharacteristicCallbacks());
  btrCharacteristic->setValue("0");
  btrCharacteristic->setCallbacks(new BatteryCharacteristicCallbacks());
  newTimeCharacteristic->setValue("0");
  newTimeCharacteristic->setCallbacks(new NewTimeCharacteristicCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE init succeed");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (runEvery(PERIOD_TX_LORA, &lora_last_tx)) {
    uint32_t time_millis = millis();
    String characteristic_msg = String(time_millis);
    char characteristic_array[22];
    characteristic_msg.toCharArray(characteristic_array, characteristic_msg.length());
    gtCharacteristic->setValue(characteristic_array);
    String message = "";
    String messageid = "";
    if(send_updated_timer == true){ // Update the timer values
      message = "{\"type\": \"timer\",";
      message += " \"current_ts\": \"" + String(millis()) + "\",";
      message += " \"end_ts\": \"" + String(timer_end_ts_shared) + "\"}";
      messageid = "Timer Update";
    }else{                          // Send default message
      // Prepare LoRa TX message
      message = "HeLoRa World! ";
      message += "I'm a Gateway! Runtime:";
      message += millis();
      messageid = "Default";
    }
    if(lora_exist){
      LoRa_sendMessage(message);    // Send a message
    }
    Serial.println("LoRa MSG: "+message);
    Serial.println("LoRaTX "+messageid);

  }
}


bool runEvery(unsigned long interval, unsigned long *previousMillis)
{
  unsigned long currentMillis = millis(); // Store the current time
  // Check if a time interval has passed since the last previousMillis
  if (currentMillis - *previousMillis >= interval)
  {
    *previousMillis = currentMillis;      // Update the time of *previousMillis
    return true;
  }
  return false;
}

String get_remaining_time(unsigned long timer_end_millis){
  unsigned long timer_current = millis();
  String timer_data = "";
  if(timer_end_millis > timer_current){
    int remainig_hours = int((timer_end_millis-timer_current)/(60 * 60 * 1000));
    String remainig_hours_str = String(remainig_hours);
    if(remainig_hours < 10) remainig_hours_str = "0" + remainig_hours_str;
    int remainig_min = int((timer_end_millis-timer_current)/(60 * 1000)) - remainig_hours * 60;
    String remainig_min_str = String(remainig_min);
    if(remainig_min < 10) remainig_min_str = "0" + remainig_min_str;
    int remainig_sec = int((timer_end_millis-timer_current)/(1000)) - remainig_hours * 60 * 60 - remainig_min * 60;
    String remainig_sec_str = String(remainig_sec);
    if(remainig_sec < 10) remainig_sec_str = "0" + remainig_sec_str;
    timer_data = remainig_hours_str + ":" + remainig_min_str + ":" + remainig_sec_str;
  }else{
    timer_data = "00:00:00";
  }
  return timer_data;
}

//########################################
//##              LoRa                  ##
//########################################

/**
 * [LoRa_rxMode Setup Lora's Receiver Mode]
 */
void LoRa_rxMode(){
  digitalWrite(LED_LORA_RX, HIGH); // turn on the LoRa RX LED
  digitalWrite(LED_LORA_TX, LOW);  // turn off the LoRa TX LED
  LoRa.disableInvertIQ();          // normal mode
  LoRa.receive();                  // set receive mode
}

/**
 * [LoRa_txMode Setup Lora's Transmitter Mode]
 */
void LoRa_txMode(){
  digitalWrite(LED_LORA_RX, LOW);  // turn off the LoRa RX LED
  digitalWrite(LED_LORA_TX, HIGH); // turn on the LoRa TX LED
  LoRa.idle();                     // set standby mode
  LoRa.enableInvertIQ();           // active invert I and Q signals
}

/**
 * [LoRa_sendMessage Transmit a String using LoRa]
 * @param message [String that will be transmitted]
 */
void LoRa_sendMessage(String message) {
  LoRa_txMode();          // set tx mode
  LoRa.beginPacket();     // start packet
  LoRa.print(message);    // add payload
  LoRa.endPacket(true);   // finish packet and send it
}

/**
 * [onReceiveLora Interrupt that activates when LoRa receives a message]
 * @param packetSize [Size of the package received]
 */
void onReceiveLora(int packetSize) {
  String message = "";            //Variable used to store the received message
  if(lora_exist){
    while (LoRa.available()) {      // Loop while there is data in the RX buffer
    message += (char)LoRa.read(); // Read a new value from the RX buffer
  }
  }
  

  Serial.print("LoRaRX Timestamp: ");
  Serial.println(millis());
  // Show the received message in serial monitor
  Serial.print("LoRaRX Received: ");
  Serial.println(message);

  // Parse message
  int init_index = 0;
  int end_index = message.indexOf(':', init_index);
  String json_param = message.substring(message.indexOf('"', init_index) + 1, end_index - 1);
  String json_value = message.substring(message.indexOf('"', end_index) + 1 , message.indexOf(',', end_index) - 1);
  uint32_t current_ts_buoy = 0;
  uint32_t new_timer_end_ts_shared = 0;
  uint32_t battery_read = 0;
  
  if(json_param.equals("type")){
    if(json_value.equals("timer")){     
      init_index = message.indexOf(',', end_index);
      end_index = message.indexOf(':', init_index);
      json_param = message.substring(message.indexOf('"', init_index) + 1, end_index - 1);
      json_value = message.substring(message.indexOf('"', end_index) + 1 , message.indexOf(',', end_index) - 1);
      if(json_param.equals("current_ts")) current_ts_buoy = strtoul(json_value.c_str(), NULL, 10);
      init_index = message.indexOf(',', end_index);
      end_index = message.indexOf(':', init_index);
      json_param = message.substring(message.indexOf('"', init_index) + 1, end_index - 1);
      json_value = message.substring(message.indexOf('"', end_index) + 1 , message.indexOf(',', end_index) - 1);
      if(json_param.equals("end_ts")) new_timer_end_ts_shared = strtoul(json_value.c_str(), NULL, 10);
      init_index = message.indexOf(',', end_index);
      end_index = message.indexOf(':', init_index);
      json_param = message.substring(message.indexOf('"', init_index) + 1, end_index - 1);
      json_value = message.substring(message.indexOf('"', end_index) + 1 , message.indexOf(',', end_index) - 1);
      if(json_param.equals("battery")) battery_read = strtoul(json_value.c_str(), NULL, 10);
      Serial.println("LoraRX Type:timer");
      
      Serial.println("LoRaRX Buoy ts:"+String(current_ts_buoy)+" ms");
      Serial.println("LoraRX End:"+String(new_timer_end_ts_shared)+" ms");
      if(send_updated_timer == true){               // The device is sending an updater timer
        Serial.println("Checking timer variables...");
        // Check if the timer has been updated in the Buoy
        if(timer_end_ts_shared == new_timer_end_ts_shared){
          // The end timestamp has been updated
          if(timer_end_ts_shared > 0) timer_end_ts_local = millis() + timer_end_ts_shared - current_ts_buoy;
          send_updated_timer = false;
          reseting_timer = false;
          Serial.println("Updated timer variables");
        }else{
          Serial.println("The timer variables haven't been updated");
        }
      }else{
        timer_end_ts_shared = new_timer_end_ts_shared;
        if(timer_end_ts_shared > 0 && timer_end_ts_local != timer_end_ts_shared) timer_end_ts_local = millis() + timer_end_ts_shared - current_ts_buoy;
      }
      String timer_data = get_remaining_time(timer_end_ts_local);     
      Serial.println("Timer Data:"+timer_data);
      gateway_doc["buoy_ts"] = current_ts_buoy;
      gateway_doc["gateway_ts"] = millis();
      gateway_doc["timer_end_ts"] = timer_end_ts_shared; 
      gateway_doc["battery"] = battery_read;
      String json_gateway_str;
      serializeJson(gateway_doc, json_gateway_str);
      Serial.println(json_gateway_str);
      String characteristic_message;
      char current_ts_array[22], battery_read_array[22], time_array[22];
      characteristic_message = String(current_ts_buoy);
      characteristic_message.toCharArray(current_ts_array, characteristic_message.length());
      characteristic_message = String(battery_read);
      characteristic_message.toCharArray(battery_read_array, characteristic_message.length());
      characteristic_message = String(timer_end_ts_shared);
      characteristic_message.toCharArray(time_array, characteristic_message.length());
      nodeCharacteristic->setValue(current_ts_array);
      btrCharacteristic->setValue(battery_read_array);
      timeCharacteristic->setValue(time_array);
    } else if(json_param.equals("gps")){
      StaticJsonDocument<256>gps_doc;
      DeserializationError error = deserializeJson(gps_doc, message);
      if(!error){
        gateway_doc["sattelites"] = gps_doc["sattelites"];
        gateway_doc["hdop"] = gps_doc["hdop"];
        gateway_doc["lat"] = gps_doc["lat"];
        gateway_doc["lng"] = gps_doc["lng"];
        gateway_doc["datetime"] = gps_doc["datetime"];
        gateway_doc["altitude"] = gps_doc["altitude"];
        gateway_doc["course_deg"] = gps_doc["course_deg"];
        gateway_doc["speed_kmph"] = gps_doc["speed_kmph"];
        gateway_doc["isValid"] = gps_doc["isValid"];
        JsonObject obj = gateway_doc.as<JsonObject>();
        String json_gateway_str;
        String gps_data;
        gps_data = obj["lat"].as<String>();
        gps_data += ";";
        gps_data += obj["lng"].as<String>();
        char  gps_msg[22];
        gps_data.toCharArray(gps_msg, gps_data.length());
        serializeJson(gateway_doc, json_gateway_str);
        Serial.println(json_gateway_str);
        gpsCharacteristic->setValue(gps_msg);
      }
    }
  }
  

  last_msg_rx = millis();         // Store the time of the last reception
}

/**
 * [onTxDoneLoRa Interrupt that activates when LoRa ends transmission]
 */
void onTxDoneLoRa() {
  Serial.println("TxDone");     // Notify the end of transmission in the serial monitor
  LoRa_rxMode();                // Activate lora's reception mode
}