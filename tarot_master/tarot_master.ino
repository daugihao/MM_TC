#include <Wire.h>

#define X_MASTER 0
#define NUMBER_OF_SLAVES 2
#define DATA_LENGTH 4
#define NUMBER_OF_CARDS 5

#define MODE 0

#define DELAY_PERIOD 1000

byte tagarray[][4] = {
  {0x5C, 0xF9, 0x9D, 0xD6}, // The Fool
  {0xCC, 0x21, 0x9D, 0xD6}, // Justice
  {0xDC, 0xCB, 0x9C, 0xD6}, // The Star
  {0xEC, 0xC5, 0x9C, 0xD6}, // Death
  {0xCC, 0x1D, 0x9D, 0xD6}, // Wheel of Fortune
};

byte tag[][4] = {0xFF, 0xFF, 0xFF, 0xFF};

void setup()
{
  Wire.begin(X_MASTER);        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start Serial for output
  Serial.println("Tarot master is ready to receive data!");
  pinMode(LED_BUILTIN, OUTPUT);

  int N_state = 0;
}

void loop()
{
  switch (MODE) {
  // DEBUGGING: Loop through asking each card reader for latest value
  case 0:
    for (int i = 1; i <= NUMBER_OF_SLAVES; i++) {
      checkTag(tag, tagarray, i);
      delay(DELAY_PERIOD);
    }
    break;
  // OPERATION: State machine for Tarot reading
  case 1:
    flash(1);
    flash(2);
    flash(3);
    flash(4);
    flash(5);
    break;
  }
  
}

void flash(int number_flashes)
{
  for (int i = 0; i < number_flashes; i++) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(DELAY_PERIOD/20);
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(DELAY_PERIOD/5);
  }
  delay(DELAY_PERIOD);
}

int checkTag(byte tagreading[][4], byte tagarray[][4], int i)
{
  int N_card = 0;
  Wire.requestFrom(i, DATA_LENGTH);
  for (int b = 0; b < DATA_LENGTH; b++) {
    tagreading[1][b] = Wire.read();
  }
  Serial.print("Card reader ");
  Serial.print(i);
  Serial.print(": ");
  Serial.print(tagreading[1][0], HEX);
  Serial.print(tagreading[1][1], HEX);
  Serial.print(tagreading[1][2], HEX);
  Serial.print(tagreading[1][3], HEX);

  // Check if no card present
  for (int b = 0; b < DATA_LENGTH; b++) {
    if (tagreading[1][b] != 0x00) {
      break;
    }
    if (b == DATA_LENGTH-1) {
      Serial.print(" - No card presented: ");
      Serial.println(N_card);
      return N_card;
    }
  }
  
  // Check if card matches
  N_card = -1;
  for (int x = 0; x < NUMBER_OF_CARDS; x++) {
    for (int b = 0; b < DATA_LENGTH; b++) {
      if (tagreading[1][b] != tagarray[x][b]) {
        break;
      } else {
        if (b == DATA_LENGTH-1) {
          N_card = x+1;
        } else {
          continue;
        }
      }
    }
    if (N_card != -1) {
      break;
    }
  }
  Serial.print(" - Card presented: ");
  Serial.println(N_card);
  return N_card;
}
      
