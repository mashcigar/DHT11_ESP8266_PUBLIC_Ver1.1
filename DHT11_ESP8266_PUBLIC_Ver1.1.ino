

/***************************************************
  Adafruit ESP8266 Sensor Module  

  This version adds a DHT-11 module and deep sleeps for 20 mins between readings and upload to MQTT.
  Sleep cycle length can be changed.
  
  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino
  Works great with Adafruit's Huzzah ESP board:
  ----> https://www.adafruit.com/product/2471
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

/**********************Libraries*********************/
#include "ESP8266WiFi.h"
#include "Wire.h"
//#include "Adafruit_HDC1000.h"
#include "DHT.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

extern "C" {
#include "user_interface.h"
}
ADC_MODE(ADC_VCC);  //for Voltage reading

/********************SENSORS***************************/
//Create HDC1008
//Adafruit_HDC1000 hdc = Adafruit_HDC1000();

//DHT11 sensor
#define DHTPIN 2
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE, 15);
/********************SLEEP MODE**********************/
const int sleepTimeS = 300;    // SleepTimeS is in microseconds or μs.
/* 60μs - 1min, 120μs- 2min, 180μs - 3min 300μs - 5min */
/****************************************************/

// WiFi parameters
#define WLAN_SSID       "...YOUR SSID...."
#define WLAN_PASS       "YOUR WIFI PASSWORD"

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "your ADAFRUIT username"
#define AIO_KEY         "Your Adafruit IO AIO Key"

// Functions
void connect ();

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, client ID, username, and password in flash memory.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;

// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM  = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

//******************************Setup Feeds ***************************************
const char TEMPERATURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/temperature";
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_FEED);

const char HUMIDITY_FEED[] PROGMEM = AIO_USERNAME "/feeds/humidity";
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_FEED);

const char VOLTAGE_FEED[] PROGMEM = AIO_USERNAME "/feeds/voltage";
Adafruit_MQTT_Publish voltage = Adafruit_MQTT_Publish(&mqtt, VOLTAGE_FEED);

/*************************** Sketch Code ************************************/
void setup() {

  // Init sensor if using HDC1008 use next line
  //hdc.begin();
  // Set SDA and SDL ports for HDC1008
  //Wire.begin(2, 14);
 
  dht.begin();       //reads data from DHT sensor
 
  Serial.begin(115200);
  Serial.println(F("IoT Weather Station"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // connect to adafruit io
  connect();



}
/*********************Main Loop******************************/
void loop() {
  // pings adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

/********************DHT Readings**************************/
  // Grab the current state of the sensor
  int humidity_data = (int)dht.readHumidity();
  // Publish data
  if (! humidity.publish(humidity_data))
    Serial.println(F("Failed to publish humidity"));
  else
    Serial.println(F("Humidity published!"));
    delay(500);
  
  int temperature_data = (int)dht.readTemperature() * 9/5 + 32;
  // Publish data
  if (! temperature.publish(temperature_data))
    Serial.println(F("Failed to publish temperature"));
  else
    Serial.println(F("Temperature published!"));
    delay(500);

  float voltage_data = (int)ESP.getVcc() /1000.0;
  // Publish data
  if (! voltage.publish(voltage_data))
    Serial.println(F("Failed to publish voltage"));
  else
    Serial.println(F("Voltage published!"));
    delay(500);

/*******************SLEEP MODE***********************/    
    Serial.println(F("ENTERING SLEEP MODE!"));
    ESP.deepSleep(sleepTimeS * 1000000); //Sleep mode, for power save
}
/**************************************************/
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));


}
