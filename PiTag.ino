/*
    PiTag.ino

    Created on: 23.06.17

    The PiTag System

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>
#include <Adafruit_NeoPixel.h>

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D2
#define RST_PIN D1
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.       // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;


#include <Hash.h>

const uint8_t LEDPIN = D8;
const String TEAMNAME = "blue";
const uint32_t LEDCOLOUR = 0x0000FF;
const String ID = "noot";
String tagName;
String state;
int blinkCount = 0;
bool blinkState = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LEDPIN, NEO_RGB + NEO_KHZ400);

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;



void notifyPointScored(String attackerId) {
  webSocket.sendTXT("pointscore:" + attackerId + "," + ID);
}

// Handle WebSocket Event
void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {


  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      state = "disconnected";
      strip.setPixelColor(0, 255, 255, 255);
      strip.show();
      delay(1000);
      strip.setPixelColor(0, 0, 0, 0);
      strip.show();
      delay(1000);
      break;
    case WStype_ERROR:
      Serial.printf("[WSc] Error: %s\n",  payload);
      break;
    case WStype_CONNECTED:
      Serial.printf("[WSc] Connected to url: %s\n",  payload);
      // send message to server when Connected
      webSocket.sendTXT("addplayer:" + ID + "," + TEAMNAME);
      state = "playing";
      strip.setPixelColor(0, LEDCOLOUR);
      strip.show();

      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);

      if ((char *)payload == "gameover") {
        state = "gameover";
        strip.setPixelColor(0, 255, 255, 255);
        strip.show();
      } else if ((char *)payload == "eliminated") {
        state = "eliminated";

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

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  
  pinMode(BUILTIN_LED, OUTPUT);

  Serial.println("Connecting to RFID");
  // RFID
  SPI.begin(); // Init SPI bus
  Serial.println("Finished");
  delay(100);
  mfrc522.PCD_Init(); // Init MFRC522 card
  Serial.println("Connected");

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  Serial.println("Connecting to Wifi");
  WiFiMulti.addAP("PITAG", "letmeinnow");
  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println(".");
    delay(100);
  }
  Serial.println("Connected. Connecting to Websocket");

  webSocket.begin("172.24.1.1", 8080);
  //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
  webSocket.onEvent(webSocketEvent);

  Serial.println("Init Completed");

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
  Serial.println("Checking for card");
  if (scanCard()) {
    for (int i; i < 100; i++) {
      Serial.println("nooting");
      delay(100);
      webSocket.loop();
    }
  }
  if (state == "eliminated") {
    blinkCount++;
    if (blinkCount == 50) {
      blinkCount = 0;
      Serial.println("Blink");
      if (blinkState) {
        strip.setPixelColor(0, 0, 0, 0);
      } else {
        strip.setPixelColor(0, LEDCOLOUR);
      }
      blinkState = !blinkState;
    }
  }
  webSocket.loop();
  strip.show();
}


