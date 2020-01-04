#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

// I2C details
#define X_SLAVE 1

// MFRC522 PIN Numbers : RESET + SDAs
#define RST_PIN 8
#define SS_PIN  9

// Create an MFRC522 instance :
MFRC522 mfrc522[1];

void setup() {

  SPI.begin();                  // Init SPI bus
  Wire.begin(X_SLAVE);          // join i2c bus (address optional for master)
  Wire.onRequest(requestEvents);

  Serial.begin(9600);           // Initialize serial communications with the PC
  Serial.print("Communication set up for Tarot slave: ");
  Serial.println(X_SLAVE);

  /* looking for MFRC522 readers */
  mfrc522[0].PCD_Init(SS_PIN, RST_PIN);
  Serial.print("MFRC522 ");
  mfrc522[0].PCD_DumpVersionToSerial();
  //mfrc522[0].PCD_SetAntennaGain(mfrc522[0].RxGain_max);
}

void loop() {

  int reader = 0;

    // Looking for new cards
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F("Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();

      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC..
}

void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void requestEvents()
{
  byte b0 = mfrc522[0].uid.uidByte[0];
  byte b1 = mfrc522[0].uid.uidByte[1];
  byte b2 = mfrc522[0].uid.uidByte[2];
  byte b3 = mfrc522[0].uid.uidByte[3];
  Serial.print("Sending data back to master: ");
  Serial.print(b0, HEX);
  Serial.print(b1, HEX);
  Serial.print(b2, HEX);
  Serial.println(b3, HEX);
  
  Wire.write(b0);
  Wire.write(b1);
  Wire.write(b2);
  Wire.write(b3);
}
