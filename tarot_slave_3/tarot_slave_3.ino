#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

// I2C details
#define X_SLAVE 3

// MFRC522 PIN Numbers : RESET + SDAs
#define RST_PIN 8
#define SS_PIN  9

// Create an MFRC522 instance :
MFRC522 mfrc522;

void setup() {
  Serial.begin(9600);           // Initialize serial communications with the PC

  SPI.begin();                  // Init SPI bus
  Wire.begin(X_SLAVE);          // join i2c bus (address optional for master)
  Wire.onRequest(requestEvent);

  /* looking for MFRC522 readers */
  mfrc522.PCD_Init(SS_PIN, RST_PIN);
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  Serial.print("MFRC522 ");
  mfrc522.PCD_DumpVersionToSerial();
}

void loop() {

    // Looking for new cards
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F("Card UID:"));
      dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println();

      // Halt PICC
      mfrc522.PICC_HaltA();
      // Stop encryption on PCD
      mfrc522.PCD_StopCrypto1();
    } //if (mfrc522.PICC_IsNewC..
}

void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void requestEvent()
{
  //Send latest read card
  Serial.print("Sending data back to master from slave ");
  Serial.print(X_SLAVE);
  Serial.print(": ");
  Serial.print(mfrc522.uid.uidByte[0], HEX);
  Serial.print(mfrc522.uid.uidByte[1], HEX);
  Serial.print(mfrc522.uid.uidByte[2], HEX);
  Serial.println(mfrc522.uid.uidByte[3], HEX);
  Wire.write(mfrc522.uid.uidByte[0]);
  Wire.write(mfrc522.uid.uidByte[1]);
  Wire.write(mfrc522.uid.uidByte[2]);
  Wire.write(mfrc522.uid.uidByte[3]);
  //Reset card tag values
  mfrc522.uid.uidByte[0] = 0;
  mfrc522.uid.uidByte[1] = 0;
  mfrc522.uid.uidByte[2] = 0;
  mfrc522.uid.uidByte[3] = 0;
}
