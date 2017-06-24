#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.       // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  Serial.println("Scan PICC to see UID and type...");

  for (byte i = 0; i < 6; i++) {
                key.keyByte[i] = 0xFF;//keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
        }

}
String tagName;
String passKey;


void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    digitalWrite(13, LOW);
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    digitalWrite(13, LOW);
    return;
  }

  Serial.println("card selected");
 
  Serial.print(F("Card UID:"));
  checkCard(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  digitalWrite(13, HIGH);
  
}

void checkCard(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    tagName.concat(String(buffer[i], HEX));
  }

  if (tagName == passKey) {
    losePoint();
  }
  }
  Serial.println();
  Serial.println(tagName);
  tagName = "";
}
