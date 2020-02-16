#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define X_MASTER 0
#define NUMBER_OF_SLAVES 6
#define DATA_LENGTH 4
#define NUMBER_OF_CARDS 5

#define MODE 0
#define TEST_SLAVE 1
#define DELAY_PERIOD 500

#define LED_PIN 6
#define NUM_PIXELS 12
#define DELAY_LED 

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

int N_card = 0;
int N_state = 0;

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
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.playFolder(1, 100);
  
  Wire.begin(X_MASTER);        // join i2c bus (address optional for master)
  Serial.println("Tarot master is ready to receive data!");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(9, INPUT);

  pixels.begin();
  pixels.show();
  pixels.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  for (int slave = 1; slave <= NUMBER_OF_SLAVES; slave++) {
      checkTag(tag, tagarray, slave);
      delay(DELAY_PERIOD);
  }
}

void loop()
{
  colorWipe(pixels.Color(255, 0, 0), 50); // Red
  colorWipe(pixels.Color(0, 255, 0), 50); // Green
  colorWipe(pixels.Color(0, 0, 255), 50); // Blue
  colorWipe(pixels.Color(255, 255, 255), 50); // White
  
  switch (MODE) {
  // DEBUGGING: Loop through asking each card reader for latest value
  case 0:
    for (int slave = 1; slave <= NUMBER_OF_SLAVES; slave++) {
      checkTag(tag, tagarray, slave);
      delay(DELAY_PERIOD);
    }
    break;
  case 1:
    checkTag(tag, tagarray, TEST_SLAVE);
    delay(DELAY_PERIOD);
    break;
  // OPERATION: State machine for Tarot reading
  case 2:
    Serial.print("State: ");
    Serial.println(N_state);
    switch (N_state) {
      case 0:
        // Introduction audio
        N_card = checkTag(tag, tagarray, 1);
        N_state = stateChange(N_state, N_card, -1);
        break;
      case 1:
        // Correct first card audio
        flash(N_state);
        N_card = checkTag(tag, tagarray, 2);
        N_state = stateChange(N_state, N_card, -1);
        break;
      case 2:
        // Correct second card audio
        flash(N_state);
        delay(5000);
        N_state = stateChange(N_state, N_card, 3);
        break;
      case 3:
        // Success audio
        flash(N_state);
        delay(5000);
        N_card = checkTag(tag, tagarray, 1);
        N_card = checkTag(tag, tagarray, 2);
        N_state = stateChange(N_state, N_card, 0);
        break;
      case 10:
        // FAILURE: Incorrect card audio
        flash(N_state);
        delay(5000);
        N_card = checkTag(tag, tagarray, 1);
        N_card = checkTag(tag, tagarray, 2);
        N_state = stateChange(N_state, N_card, 0);
        break;
    }
    break;
  }
  delay(DELAY_PERIOD);
}

void flash(int number_flashes)
{
  if (number_flashes < 0) {
    number_flashes = 0;
  }
  for (int flash = 0; flash < number_flashes; flash++) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(DELAY_PERIOD/20);
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(DELAY_PERIOD/5);
  }
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
  N_card = 10;
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
    if (N_card != 10) {
      break;
    }
  }
  Serial.print(" - Card presented: ");
  Serial.println(N_card);
  return N_card;
}

int stateChange(int state, int card, int force)
{ 
  if (force >= 0) {
    state = force;
    if (state == 0) {
      myDFPlayer.playFolder(1, 100);
    } else {
    myDFPlayer.playFolder(1, state);
    }
  }
  if (card == 0x0 || card == 0xFFFFFF) { // No card is shown - stay where you are
    return state;
  }
  if (card == state + 1) { // Card is incremental card - increase state by one
    state = state + 1;
    myDFPlayer.playFolder(1, state);
    return state;
  } else {
    state = 10;
    myDFPlayer.playFolder(1, state);
    return state;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}
