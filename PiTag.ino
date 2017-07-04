/*
    PiTag.ino

    Created on: 23.06.17

    The PiTag System

*/


// Include Needed Libraries
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>
#include <Adafruit_NeoPixel.h>

#include <SPI.h>
#include <MFRC522.h>
#include <Hash.h>


// The pins to connect to
#define SS_PIN D2
#define RST_PIN D1
#define LED_PIN D3

// **CHANGE ONLY THESE THINGS**

// The name of the team(can only be blue or red)
const String TEAMNAME = "blue";
// The colour of the team(controls the led)
const uint32_t TEAMCOLOUR = 0x0000FF;
// The ID of the card(must be lowercase, player's own card)
const String ID = "c151b21";
// The wifi AP's ssid.
const String SSID = "PITAG";
// The wifi AP's password.
const String PASS = "letmeinnow";


String tagName = "";
String state;
int blinkCount = 0;
bool blinkState = false;

// Initialises MRFC522 board(RFID)
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initialises the neopixel.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_PIN, NEO_RGB + NEO_KHZ400);

// Initialises wifi communication things.
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

// Called when tagged.
void notifyPointScored(String attackerId) {
  // Notify server of event.
  webSocket.sendTXT("pointscore:" + attackerId + "," + ID);
}

// Handle WebSocket Event
void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      // The board has been disconnected from server.
      Serial.printf("[WSc] Disconnected!\n");
      state = "disconnected";
      // Flashes pixel white.
      strip.setPixelColor(0, 255, 255, 255);
      strip.show();
      delay(1000);
      strip.setPixelColor(0, 0, 0, 0);
      strip.show();
      delay(1000);
      break;
    case WStype_ERROR:
      // Prints if error.
      Serial.printf("[WSc] Error: %s\n",  payload);
      break;
    case WStype_CONNECTED:
      // When connected
      Serial.printf("[WSc] Connected to url: %s\n",  payload);
      // tells server who this player is
      webSocket.sendTXT("addplayer:" + ID + "," + TEAMNAME);
      state = "playing";
      // Set indicator LED
      strip.setPixelColor(0, TEAMCOLOUR);
      strip.show();

      break;
    case WStype_TEXT:
      // Debug Statements
      Serial.printf("[WSc] get text: %s\n", payload);
      String str = (char*)payload;
      // If gets gameover event
      if (str == "gameover") {
        // Set pixel to solid white.
        state = "gameover";
        strip.setPixelColor(0, 255, 255, 255);
        strip.show();
      } else if (str == "eliminated") {
        // If player is eliminated, set state
        state = "eliminated";

      }
      break;
  }

}
// Do beginning Stuffs
void setup() {
  // Serial.begin(921600);
  Serial.begin(9600);

  // Set LED to off
  strip.begin();
  strip.show();
  
  // Using builtin led just in case.
  pinMode(BUILTIN_LED, OUTPUT);

  Serial.println("Connecting to RFID");
  // RFID
  SPI.begin(); // Init SPI bus
  delay(100);
  mfrc522.PCD_Init(); // Init MFRC522 card
  Serial.println("Connected");

  Serial.println();
  Serial.println();
  Serial.println();

  // Wait for seperate wifi chip in esp to boot.
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  Serial.println("Connecting to Wifi");
  // Connect to the access point.
  WiFiMulti.addAP(SSID, PASS);
  // Wait to connect.
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println(".");
    delay(100);
  }
  Serial.println("Connected. Connecting to Websocket");

  // Connect to server
  webSocket.begin("172.24.1.1", 8080);
  // Set event callback.
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
  // Check card for relevance.
  checkCard(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  digitalWrite(BUILTIN_LED, HIGH);
  return true;

}

void checkCard(byte *buffer, byte bufferSize) {
  // Form a string with the card's ID
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    tagName.concat(String(buffer[i], HEX));
  }

  // Notify Server
  notifyPointScored(tagName);
  tagName = "";
}

void loop() {
  Serial.println("Checking for card");
  // Check for card
  if (scanCard()) {
    for (int i=0; i < 100; i++) {
      // Cooldown period.
      Serial.println("nooting");
      delay(100);
      webSocket.loop();
    }
  }
  // If player is eliminated, flash LED
  if (state == "eliminated") {
    blinkCount++;
    if (blinkCount == 50) {
      blinkCount = 0;
      if (blinkState) {
        strip.setPixelColor(0, 0, 0, 0);
      } else {
        strip.setPixelColor(0, TEAMCOLOUR);
      }
      blinkState = !blinkState;
    }
  }
  webSocket.loop();
  strip.show();
}


