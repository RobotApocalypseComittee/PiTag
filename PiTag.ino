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

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 4
#define RST_PIN 5
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.       // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;


#include <Hash.h>

const uint8_t LEDPIN = D0;
const String TEAMNAME = "blue";
const uint32_t LEDCOLOUR = 0x00FF00;
const String ID = "noot";
String tagName;

Adafruit_NeoPixel strip;

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;



void notifyPointScored(String attackerId) {
  webSocket.sendTXT("pointscore:"+attackerId+","+ID);
}

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
                webSocket.sendTXT("addplayer:"+ID+","+"blue");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[WSc] get text: %s\n", payload);
            
            if ((char *)payload == "gameover") {
              strip.setPixelColor(0, 255, 255, 255);
            }

      // send message to server
      // webSocket.sendTXT("message here");
            break;
    }

}
// Do beginning Stuffs
void setup() {
    // Serial.begin(921600);
    Serial.begin(9600);

    Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LEDPIN, NEO_RGB + NEO_KHZ800);
    strip.begin();
    strip.setPixelColor(0, 0, 0, 255);
    strip.show(); // Initialize all pixels to 'off'

    pinMode(BUILTIN_LED, OUTPUT);

    //Serial.setDebugOutput(true);
    Serial.setDebugOutput(true);

    // RFID
    SPI.begin();      // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    Serial.println();
    Serial.println();
    Serial.println();

      for(uint8_t t = 4; t > 0; t--) {
          Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
          Serial.flush();
          delay(1000);
      }

    WiFiMulti.addAP("PITAG", "letmeinnow");

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.begin("172.24.1.1", 8080);
    //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
    webSocket.onEvent(webSocketEvent);

}

bool scanCard() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    digitalWrite(BUILTIN_LED, LOW);
    return false;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    digitalWrite(BUILTIN_LED, LOW);
    return false;
  }

 
  Serial.print(F("Card UID:"));
  checkCard(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  digitalWrite(BUILTIN_LED, HIGH);
  return true;
  
}

void checkCard(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    tagName.concat(String(buffer[i], HEX));
  }

  notifyPointScored(tagName);
  tagName = "";
}

void loop() {
  if (scanCard()) {
    for (int i; i<100;i++) {
      delay(100);
      webSocket.loop();
    }
  }
  webSocket.loop();
}


