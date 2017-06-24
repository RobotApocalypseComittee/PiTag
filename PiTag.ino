/*
 *  PiTag.ino
 *
 *  Created on: 23.06.17
 *  
 *  The PiTag System
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>
#include <Adafruit_NeoPixel.h>


#include <Hash.h>

const uint8_t LEDPIN = D0;
const String TEAMNAME = "blue";
const uint32_t LEDCOLOUR = 0x00FF00;
const String ID;

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

String playerId = ""
String teamName = "blue";


// Handle WebSocket Event
void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {


    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                Serial.printf("[WSc] Connected to url: %s\n",  payload);
                // send message to server when Connected
                webSocket.sendTXT("addplayer,"+ID+","+"blue");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[WSc] get text: %s\n", payload);

      // send message to server
      // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[WSc] get binary lenght: %u\n", lenght);
            hexdump(payload, lenght);

            // send data to server
            // webSocket.sendBIN(payload, lenght);
            break;
    }

}
// Do beginning Stuffs
void setup() {
    // Serial.begin(921600);
    Serial.begin(9600);

    //Serial.setDebugOutput(true);
    Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

      for(uint8_t t = 4; t > 0; t--) {
          Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
          Serial.flush();
          delay(1000);
      }

    WiFiMulti.addAP("SSID", "passpasspass");

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.begin("192.168.0.123", 81);
    //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
    webSocket.onEvent(webSocketEvent);

}

void loop() {
    webSocket.loop();
}

