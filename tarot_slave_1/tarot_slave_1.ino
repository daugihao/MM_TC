#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

// I2C details
#define X_SLAVE 1

// MFRC522 PIN Numbers : RESET + SDAs
#define RST_PIN 8
#define SS_PIN  9

// List of Tags UIDs that are allowed to open the puzzle
byte tagarray[][4] = {
  {0x5C, 0xF9, 0x9D, 0xD6},
  {0xCC, 0x21, 0x9D, 0xD6}, 
  {0xDC, 0xCB, 0x9C, 0xD6},
  {0xEC, 0xC5, 0x9C, 0xD6},
  {0xCC, 0x1D, 0x9D, 0xD6},
};

// Inlocking status :
int tagcount = 0;
bool access = false;

#define NR_OF_READERS   1

// Create an MFRC522 instance :
MFRC522 mfrc522[1];

/**
   Initialize.
*/
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

/*
   Main loop.
*/

void loop() {

  int reader = 0;

    // Looking for new cards
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F("Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();

      for (int x = 0; x < sizeof(tagarray); x++)                  // tagarray's row
      {
        for (int i = 0; i < mfrc522[reader].uid.size; i++)        //tagarray's columns
        {
          if ( mfrc522[reader].uid.uidByte[i] != tagarray[x][i])  //Comparing the UID in the buffer to the UID in the tag array.
          {
            DenyingTag();
            break;
          }
          else
          {
            if (i == mfrc522[reader].uid.size - 1)                // Test if we browesed the whole UID.
            {
              AllowTag();
            }
            else
            {
              continue;                                           // We still didn't reach the last cell/column : continue testing!
            }
          }
        }
        if (access) break;                                        // If the Tag is allowed, quit the test.
      }


      if (access)
      {
        if (tagcount == NR_OF_READERS)
        {
          CorrectTag();
        }
        else
        {
          MoreTagsNeeded();
        }
      }
      else
      {
        UnknownTag();
      }
      /*Serial.print(F("PICC type: "));
        MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
        Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));*/
      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC..
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printTagcount() {
  Serial.print("Tag n°");
  Serial.println(tagcount);
}

void DenyingTag()
{
  tagcount = tagcount;
  access = false;
}

void AllowTag()
{
  tagcount = tagcount + 1;
  access = true;
}

void Initialize()
{
  tagcount = 0;
  access = false;
}

void CorrectTag()
{
  Serial.println("Correct card presented!");
  Initialize();
}

void MoreTagsNeeded()
{
  printTagcount();
  Serial.println("System needs more cards");
  access = false;
}

void UnknownTag()
{
  Serial.println("This Tag isn't allowed!");
  printTagcount();
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
