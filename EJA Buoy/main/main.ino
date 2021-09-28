#include "main.hpp"

// The TinyGPS++ object
TinyGPSPlus gps;

// Time (millis) counters for timed operations
unsigned long last_msg_rx = 0;
unsigned long last_lora_tx = 0;
unsigned long last_erase_buffers = 0;
unsigned long last_ap_request = 0;
uint32_t pwm_move = PWM_MOVE;

// JSON with the relevant data
String gps_json;    // GPS data
String timer_json;  // Timer data

//Variables from the GPS
String gps_sattelites;
String gps_hdop;
String gps_lat;
String gps_lng;
String gps_age;
String gps_datetime;
String gps_altitude_meters;
String gps_course_deg;
String gps_speed_kmph;
String gps_course;
String gps_isvalid;
String gps_chars_processed;
String gps_sentences_with_fix;
String gps_failed_checksum;

// Variables of the timer
int timer_end_hours = 0;
int timer_end_min = 0;
int timer_end_sec = 0;
unsigned long timer_end_ts_shared = 0;
unsigned long timer_end_ts_local = 0;
unsigned long timer_current = 0;

int message_selector = GPS_MESSAGE_ID;

bool timer_reached_end = false;

//function prototypes

static void smartDelay(unsigned long ms);
bool runEvery(unsigned long interval, unsigned long *previousMillis);
static String getFloatGPS(float val, bool valid, int len, int prec);
static String getIntGPS(unsigned long val, bool valid, int len);
static String getDateTimeGPS(TinyGPSDate &d, TinyGPSTime &t);
static String getStrGPS(const char *str, int len);
void LoRa_rxMode();
void LoRa_txMode();
void LoRa_sendMessage(String message);
void onReceiveLora(int packetSize);
void onTxDoneLoRa(); 
void setup() {
  // Initialize serial communication (used for the GPS)
  Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  // Initialize pwm channel and servo motor pin
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(MOTOR1, PWM_CHANNEL);
  // Setup LoRa module
  LoRa.setPins(CS_LORA, RST_LORA, IRQ_LORA);
 

  // Initialize the LoRa module
  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.onReceive(onReceiveLora);    // Config LoRa RX routines
  LoRa.onTxDone(onTxDoneLoRa);      // Config LoRa TX routines
  LoRa_rxMode();                // Activate LoRa RX
}

void loop() {
  // put your main code here, to run repeatedly:
  // Get the GPS data
  gps_sattelites = getIntGPS(gps.satellites.value(), gps.satellites.isValid(), 5);
  gps_hdop = getIntGPS(gps.hdop.value(), gps.hdop.isValid(), 5);
  gps_lat = getFloatGPS(gps.location.lat(), gps.location.isValid(), 11, 6);
  gps_lng = getFloatGPS(gps.location.lng(), gps.location.isValid(), 12, 6);
  gps_age = getIntGPS(gps.location.age(), gps.location.isValid(), 5);
  gps_datetime = getDateTimeGPS(gps.date, gps.time);
  gps_altitude_meters = getFloatGPS(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  gps_course_deg = getFloatGPS(gps.course.deg(), gps.course.isValid(), 7, 2);
  gps_speed_kmph = getFloatGPS(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  gps_isvalid = getStrGPS(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.value()) : "*** ", 6);
  gps_chars_processed = getIntGPS(gps.charsProcessed(), true, 6);
  gps_sentences_with_fix = getIntGPS(gps.sentencesWithFix(), true, 10);
  gps_failed_checksum = getIntGPS(gps.failedChecksum(), true, 9);
  // Create a JSON with the GPS data
  gps_json = "{\"type\": \"gps\",";
  gps_json += " \"sattelites\": \""+ gps_sattelites +"\",";
  gps_json += " \"hdop\": \""+ gps_hdop +"\",";
  gps_json += " \"lat\": \""+ gps_lat +"\",";
  gps_json += " \"lng\": \""+ gps_lng +"\",";
  gps_json += " \"age\": \""+ gps_age +"\",";
  gps_json += " \"datetime\": \""+ gps_datetime +"\",";
  gps_json += " \"altitude\": \""+ gps_altitude_meters +"m\",";
  gps_json += " \"course_deg\": \""+ gps_altitude_meters +"deg\",";
  gps_json += " \"speed_kmph\": \""+ gps_speed_kmph +"Kmph\",";
  gps_json += " \"isValid\": \""+ gps_isvalid+"\"}";

  // Create a JSON with the timer data
  timer_json = "{\"type\": \"timer\",";
  timer_json += " \"current_ts\": \"" + String(millis()) + "\",";
  timer_json += " \"end_ts\": \"" + String(timer_end_ts_shared) + "\",";
  timer_json += " \"battery\": \"" + String(analogRead(BATTERY_PIN)) + "\",";
  timer_json += " \"timer_reached_end\": " + String(int(timer_reached_end)) + "}";
  // Apply delay that ensures that the gps object is being "fed".
  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10){
    //no gps data received
  }
  if (runEvery(PERIOD_TX_LORA, &last_lora_tx)) { // repeat every 1000 millis
    switch (message_selector) {
      case GPS_MESSAGE_ID:
        LoRa_sendMessage(gps_json);             // Send a message
        message_selector = TIMER_MESSAGE_ID;
        break;
      case TIMER_MESSAGE_ID:
        LoRa_sendMessage(timer_json);             // Send a message
        message_selector = GPS_MESSAGE_ID;
        break;
      default:
        // statements
        break;
    }
  }
  if((timer_end_ts_local > 0) && (millis() > timer_end_ts_local)){
    timer_reached_end = true;
    // ------   Activate release mechanism  --------------
    if(pwm_move >= 25){
      pwm_move = PWM_MOVE;
    }
    ledcWrite(PWM_CHANNEL, pwm_move);
    pwm_move += 1;
    // ---------------------------------------------------
  }
}
boolean runEvery(unsigned long interval, unsigned long *previousMillis)
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

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } while (millis() - start < ms);
}

static String getFloatGPS(float val, bool valid, int len, int prec)
{
  String result = "";
  if (!valid)
  {
    while (len-- > 1)
      result += "*";
      //Serial.print('*');
    //Serial.print(' ');
    result += " ";
  }
  else
  {
    result += String(val) + " " + String(prec);
    //Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      //Serial.print(' ');
      result += " ";
  }
  smartDelay(0);
  return result;
}

static String getIntGPS(unsigned long val, bool valid, int len)
{
  String result = "";
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len-1] = ' ';
  //Serial.print(sz);
  result += "" + String(sz);
  smartDelay(0);
  return result;
}

static String getDateTimeGPS(TinyGPSDate &d, TinyGPSTime &t){
  String result = "";
  if (!d.isValid())
  {
    //Serial.print(F("********** "));
    result += "********** ";
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    //Serial.print(sz);
    result += "" + String(sz);
  }

  if (!t.isValid())
  {
    //Serial.print(F("******** "));
    result += "******** ";
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    result += "" + String(sz);
  }

  result += getIntGPS(d.age(), d.isValid(), 5);
  smartDelay(0);
  return result;
}

static String getStrGPS(const char *str, int len)
{
  String result;
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    //Serial.print(i<slen ? str[i] : ' ');
    if(i<slen){
      result += str[i];
    }else{
      result += ' ';
    }
  smartDelay(0);
  return result;
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
  LoRa.enableInvertIQ();           // active invert I and Q signals
  LoRa.receive();                  // set receive mode
}

void LoRa_txMode(){
  digitalWrite(LED_LORA_RX, LOW);  // turn off the LoRa RX LED
  digitalWrite(LED_LORA_TX, HIGH); // turn on the LoRa TX LED
  LoRa.idle();                     // set standby mode
  LoRa.disableInvertIQ();          // normal mode
}

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
  String message = "";              //Variable used to store the received message

  while (LoRa.available()) {        // Loop while there is data in the RX buffer
    message += (char)LoRa.read();   // Read a new value from the RX buffer
  }

  // Parse Type of Message
  int init_index = 0;
  int end_index = message.indexOf(':', init_index);
  String json_param = message.substring(message.indexOf('"', init_index) + 1, end_index - 1);
  String json_value = message.substring(message.indexOf('"', end_index) + 1 , message.indexOf(',', end_index) - 1);
  unsigned long onboard_gateway_current_time = 0;
  
  if(json_param.equals("type")){
    if(json_value.equals("timer")){  // Parse timer message   
      init_index = message.indexOf(',', end_index);
      end_index = message.indexOf(':', init_index);
      json_param = message.substring(message.indexOf('"', init_index) + 1, end_index - 1);
      json_value = message.substring(message.indexOf('"', end_index) + 1 , message.indexOf(',', end_index) - 1);
      if(json_param.equals("current_ts")) onboard_gateway_current_time = strtoul(json_value.c_str(), NULL, 10);
      init_index = message.indexOf(',', end_index);
      end_index = message.indexOf(':', init_index);
      json_param = message.substring(message.indexOf('"', init_index) + 1, end_index - 1);
      json_value = message.substring(message.indexOf('"', end_index) + 1 , message.indexOf(',', end_index) - 1);
      if(json_param.equals("end_ts")){
        timer_end_ts_shared = strtoul(json_value.c_str(), NULL, 10);
        if(timer_end_ts_shared > 0 && timer_end_ts_local != timer_end_ts_shared)timer_end_ts_local = millis() + timer_end_ts_shared - onboard_gateway_current_time;
      }
      timer_reached_end = false;
    } 
  }
  // Store the message in the string with all messages associated to lora

  last_msg_rx = millis();           // Store the time of the last reception
}

/**
 * [onTxDoneLoRa Interrupt that activates when LoRa ends transmission]
 */
void onTxDoneLoRa() {// Store a message from the Lora process
  LoRa_rxMode();                // Activate lora's reception mode
}