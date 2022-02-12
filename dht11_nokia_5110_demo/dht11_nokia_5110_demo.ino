#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <ArduinoJson.h>

// https://weatherdbi.herokuapp.com/data/weather/jodhpur

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

ESP8266WiFiMulti WiFiMulti;

/* Declare LCD object for SPI
  Adafruit_PCD8544(CLK,DIN,D/C,CE,RST);*/
Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 5, 15, 4); /*D5, D7, D1, D8, D2 */
int contrastValue = 60; /* Default Contrast Value */

#include "DHTesp.h" // Click here to get the library: http://librarymanager/All#DHTesp

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

DHTesp dht;


// System Define Functions

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  String thisBoard = ARDUINO_BOARD;
  Serial.println(thisBoard);

  // Autodetect is not working reliable, don't use the following line
  // dht.setup(17);
  // use this instead:
  dht.setup(0, DHTesp::DHT11); // Connect DHT sensor to GPIO 17

  SetupDisplay();
  SetupWiFi();

}

void loop()
{

  // add function to get data from DHT 11 and display in screen
 
  getLocalWeatherStationDetails();
  //ClearDisplay();

  getWeatherStationDetails("Jodhpur");
  //ClearDisplay();
  getWeatherStationDetails("Mumbai");
  // ClearDisplay();
  getWeatherStationDetails("Delhi");
  // ClearDisplay();
  getWeatherStationDetails("Goa");
  delay(1000);
  ClearDisplay();

}

// User Define Functions

void SetupDisplay()
{
  /* Initialize the Display*/
  display.begin();
  display.setCursor(0, 0);
  /* Change the contrast using the following API*/
  display.setContrast(contrastValue);

}

void SetupWiFi()
{
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();

    DisplayInScreen("[SETUP] WAIT WiFi Setup...", 5);

    delay(1000);
    ClearDisplay();

  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Airtel_9711562660", "air67624");
  WiFiMulti.addAP("Tatiya bichu", "air32508");

}


void ClearDisplay()
{

  /* Clear the buffer */
  display.clearDisplay();
  display.display();
  //delay(500);

}

void DisplayInScreen(String sData, int iUser)
{
  int iPos = 1;
  /* Now let us display some text */
  display.setTextSize(1);
  display.setTextColor(BLACK);
  

  if (iUser == 1) {
    iPos = 1;
  }
  else if (iUser == 2)
  {
    iPos = 2;
  }
  else if (iUser == 3)
  {
    iPos = 3;
  }
  else
  {
    iPos = 4;
  }

  // display.setCursor(15, iPos);
  display.println(sData);
  display.display();
  delay(500);

}


void getLocalWeatherStationDetails() {
  // Get DHT11 data and display in LCD screen
  delay(dht.getMinimumSamplingPeriod());
  String sWeatherData = "";
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");

  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);


  //sWeatherData = "Local DHT11: Humidity (%)\tTemperature\n";
  sWeatherData =  "DHT11:" + String(humidity, 0) + "% :" + String(temperature, 0);
  DisplayInScreen(sWeatherData, 1);
  //delay(2000);
  //return sWeatherData;
}

void getWeatherStationDetails(String sLoction) {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    //WiFiClient client;

    HTTPClient https;

    Serial.print("[HTTP] begin...\n");
    if (https.begin(*client, "https://weatherdbi.herokuapp.com/data/weather/" + sLoction)) { // HTTPS


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          // Serial.println(payload);
          ParseJsonData(payload,sLoction);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  delay(5000);
}

void ParseJsonData(String sJSONData, String sLocation)
{

  //char json[] = sJSONData;
  String sWeatherData = "";
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, sJSONData);
  String region = sLocation;//doc["region"];
  String Temp = doc["currentConditions"]["temp"]["c"]; //+ "C : " ;
  //+ doc["currentConditions"][1][1]  +"F";
  String humidity = doc["currentConditions"]["humidity"] ;
  Serial.println("region\tHumidity (%)\tTemperature (C)\t(F))");
  //Serial.println(doc["currentConditions"]);
  Serial.print(region);
  Serial.print("\t");
  Serial.print(humidity);
  Serial.print("\t");
  Serial.print(Temp);
  Serial.println("");

  //sWeatherData = "region\tHumidity (%)\tTemperature\n";
  sWeatherData = region + ":" + humidity + ":" + Temp;
  //ClearDisplay();
  Serial.println(sWeatherData);
  DisplayInScreen(sWeatherData, 1);
  Serial.println("");
}
