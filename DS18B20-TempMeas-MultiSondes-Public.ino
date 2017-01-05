 /*
  Created by Igor Jarc
  Updated by P Becquet, for multi probes, and Jeedom use
  
 See http://iot-playground.com for details
 Please use community fourum on website do not contact author directly
 
 Code based on https://github.com/DennisSc/easyIoT-ESPduino/blob/master/sketches/ds18b20.ino
 
 External libraries:
 - https://github.com/adamvr/arduino-base64
 - https://github.com/milesburton/Arduino-Temperature-Control-Library
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//AP definitions
#define AP_SSID "LaPomme"
#define AP_PASSWORD "put your pass"
#define AP_NAME "floor2"

// EasyIoT server definitions
#define EIOT_USERNAME    "admin"
#define EIOT_PASSWORD    "pwd"
#define EIOT_IP_ADDRESS  "192.168.0.xx"
#define EIOT_PORT        80
#define EIOT_NODE        "N13S0"
#define EIOT_APIKEY      "YourAPIKey"
#define EIOT_NODE_ID1     "33"
#define EOIT_NODE_ID2     "47"
#define MAX_SONDES         2     // To be reUsed in loops.. Must no exceed number of values in sondesArray

#define REPORT_INTERVAL 60 // in sec

#define ONE_WIRE_BUS 2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

String sondesArray[] = {EIOT_NODE_ID1, EOIT_NODE_ID2}; 
float tempVals[MAX_SONDES];


#define USER_PWD_LEN 40
char unameenc[USER_PWD_LEN];
float oldTemp[MAX_SONDES];

void wifiConnect()
{
    Serial.print("Connecting to AP");
    WiFi.begin(AP_SSID, AP_PASSWORD);
    WiFi.hostname(AP_NAME);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");  
}

void sendTemperature(float temp, String NumSonde)
{  
   WiFiClient client;
   
   while(!client.connect(EIOT_IP_ADDRESS, EIOT_PORT)) {
    Serial.println("connection failed");
    wifiConnect(); 
  }

  // Sample url : http://#IP_JEEDOM#/core/api/jeeApi.php?apikey=#APIKEY#&type=virtual&id=#ID#&value=#value#
 
  String url = "";
  url += "/core/api/jeeApi.php?apikey="+String(EIOT_APIKEY)+"&type=virtual&id="+NumSonde+"&value="+String(temp); // generate EasIoT server node URL

  Serial.print("POST data to URL: ");
  Serial.println(url);
  
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(EIOT_IP_ADDRESS) + "\r\n" + 
               "Connection: close\r\n" + 
      //         "Authorization: Basic " + unameenc + " \r\n" + 
               "Content-Length: 0\r\n" + 
               "\r\n");

  delay(100);
    while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  // print your WiFi shield's IP address:
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.println("Connection closed");
}

void setup() {
  Serial.begin(115200);
  
  wifiConnect();
  for (int i=0;i<MAX_SONDES;i++) {
    oldTemp[i] = -1;
  } 
}

void loop() {
  float temp;
  int cnt = REPORT_INTERVAL;
  for (int i=0;i<MAX_SONDES;i++) {
    
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(i);
    Serial.print("Temperature: ");
    Serial.println(temp);

    if (temp != 85.0 && temp != (-127.0)) 
    {
      if (temp != oldTemp[i])
      {
        sendTemperature(temp, sondesArray[i]);
        oldTemp[i] = temp;
      }
    }
  }
  while(cnt--) delay(1000);
}


