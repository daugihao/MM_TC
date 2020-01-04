#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

// I2C details
int value = 27;
int x_slave = 1;

// MFRC522 PIN Numbers : RESET + SDAs
#define RST_PIN         8
#define SS_PIN        9

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
int readcount = 10;

#define NR_OF_READERS   1

byte ssPins[] = {SS_PIN};

// Create an MFRC522 instance :
MFRC522 mfrc522[NR_OF_READERS];

/**
   Initialize.
*/
void setup() {

  SPI.begin();                  // Init SPI bus
  Wire.begin(x_slave);          // join i2c bus (address optional for master)
  Wire.onRequest(requestEvents);

  Serial.begin(9600);           // Initialize serial communications with the PC
  while (!Serial);              // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  Serial.print("Communication set up for Tarot slave: ");
  Serial.println(x_slave);

  /* looking for MFRC522 readers */
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
    Serial.print("MFRC522 ");
    mfrc522[reader].PCD_DumpVersionToSerial();
    //mfrc522[reader].PCD_SetAntennaGain(mfrc522[reader].RxGain_max);
  }
}

/*
   Main loop.
*/

void loop() {

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {

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
  } //for(uint8_t reader..
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
  readcount++;
  Serial.println(readcount);
  Wire.write(readcount);
  Wire.write(readcount);
  Wire.write(readcount);
  Wire.write(readcount);
  Serial.println("Sending data back to master!");
}
