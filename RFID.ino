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


int block=2;//this is the block number we will write into and then read. Do not write into 'sector trailer' block, since this can make the block unusable.

byte blockcontent[16] = {"Friendly_Tea____"};//an array with 16 bytes to be written into one of the 64 card blocks is defined
byte clearblockcontent[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//all zeros. This can be used to delete a block.
byte readbackblock[18];//This array is used for reading out a block. The MIFARE_Read method requires a buffer that is at least 18 bytes to hold the 16 bytes of a block.
String tagName;

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

  // Dump debug info about the card. PICC_HaltA() is automatically called.
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  Serial.println("card selected");
 
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  digitalWrite(13, HIGH);
  
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    tagName.concat(String(buffer[i], HEX));
  }
  Serial.println();
  Serial.println(tagName);
  tagName = "";
}
