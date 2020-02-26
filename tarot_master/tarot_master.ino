#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define X_MASTER 0
#define NUMBER_OF_SLAVES 6
#define DATA_LENGTH 4
#define NUMBER_OF_CARDS 6

#define MODE 2
#define TEST_SLAVE 1
#define DELAY_PERIOD 500
#define INTRO_REPEAT_TIME 40

#define LED_PIN 6
#define NUM_PIXELS 12
#define NORMAL_WAIT 100
#define REVEAL_WAIT 100

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

int N_card = 0;
int N_state = 0;
int intro_timer = 0;

byte tagarray[][4] = {
  {0x7C, 0x60, 0x9D, 0xD6}, // 10 of cups
  {0x5C, 0x64, 0x9D, 0xD6}, // 4 of disks
  {0x3C, 0xB6, 0x9D, 0xD6}, // Knight of swords
  {0x4C, 0xF9, 0x9D, 0xD6}, // Prince of cups
  {0x5C, 0x4B, 0x9E, 0xD6}, // 9 of cups
  {0x0C, 0x86, 0x9E, 0xD6}  // The fool
};

byte tag[][4] = {0xFF, 0xFF, 0xFF, 0xFF};

void setup()
{
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);
  delay(2000);

  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  while (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println();
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    delay(1000);
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.playFolder(1, 100);
  
  Wire.begin(X_MASTER);        // join i2c bus (address optional for master)
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(9, INPUT);

  pixels.begin();
  pixels.show();
  pixels.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  for (int slave = 1; slave <= NUMBER_OF_SLAVES; slave++) {
      checkTag(tag, tagarray, slave, false);
      delay(DELAY_PERIOD);
  }

  Serial.println("Tarot master is ready to receive data!");
}

void loop()
{ 
  switch (MODE) {
  // DEBUGGING: Loop through asking each card reader for latest value
  case 0:
    for (int slave = 1; slave <= NUMBER_OF_SLAVES; slave++) {
      checkTag(tag, tagarray, slave, true);
      delay(DELAY_PERIOD);
    }
    break;
  case 1:
    checkTag(tag, tagarray, TEST_SLAVE, true);
    delay(DELAY_PERIOD);
    break;
  // OPERATION: State machine for Tarot reading
  case 2:
    Serial.print("State: ");
    Serial.println(N_state);
    switch (N_state) {
      case 0:
        // Introduction audio
        intro_timer++;
        if (intro_timer >= INTRO_REPEAT_TIME) {
          N_state = stateChange(N_state, N_card, 0);
          intro_timer = 0;
          break;
        }
        N_card = checkTag(tag, tagarray, N_state+1, true);
        N_state = stateChange(N_state, N_card, -1);
        break;
      case 1:
        // Correct first card audio
        flash(N_state);
        N_card = checkTag(tag, tagarray, N_state+1, true);
        N_state = stateChange(N_state, N_card, -1);
        break;
      case 2:
        // Correct second card audio
        flash(N_state);
        N_card = checkTag(tag, tagarray, N_state+1, true);
        N_state = stateChange(N_state, N_card, -1);
        break;
      case 3:
        // Correct third card audio
        flash(N_state);
        N_card = checkTag(tag, tagarray, N_state+1, true);
        N_state = stateChange(N_state, N_card, -1);
        break;
      case 4:
        // Correct fourth card audio
        flash(N_state);
        N_card = checkTag(tag, tagarray, N_state+1, true);
        N_state = stateChange(N_state, N_card, -1);
        break;
      case 5:
        // Correct fifth card audio
        flash(N_state);
        N_card = checkTag(tag, tagarray, N_state+1, true);
        N_state = stateChange(N_state, N_card, -1);
        break;
      case 6:
        // Correct sixth card audio
        flash(N_state);
        delay(5000);
        N_state = stateChange(N_state, N_card, 7);
        break;
      case 7:
        // Success audio
        flash(N_state);
        delay(5000);
        revealColor();
        delay(2000);
        revealColor();
        delay(10000);
        N_state = stateChange(N_state, N_card, 0);
        break;
      case 10:
        // FAILURE: Incorrect card audio
        flash(N_state);
        delay(5000);
        N_state = stateChange(N_state, N_card, 0);
        break;
    }
    break;
  }
  colorWipe(pixels.Color(255, 255, 255), NORMAL_WAIT); // White
  colorWipe(pixels.Color(0, 0, 0), NORMAL_WAIT); // White
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

int checkTag(byte tagreading[][4], byte tagarray[][4], int i, bool show_output)
{
  int N_card = 0;
  Wire.requestFrom(i, DATA_LENGTH);
  for (int b = 0; b < DATA_LENGTH; b++) {
    tagreading[1][b] = Wire.read();
  }
  if (show_output) {
    Serial.print("Card reader ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(tagreading[1][0], HEX);
    Serial.print(tagreading[1][1], HEX);
    Serial.print(tagreading[1][2], HEX);
    Serial.print(tagreading[1][3], HEX);
  }

  // Check if no card present
  for (int b = 0; b < DATA_LENGTH; b++) {
    if (tagreading[1][b] != 0x00) {
      break;
    }
    if (b == DATA_LENGTH-1) {
      if (show_output) {
        Serial.print(" - No card presented: ");
        Serial.println(N_card);
      }
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
  if (show_output) {
    Serial.print(" - Card presented: ");
    Serial.println(N_card);
  }
  return N_card;
}

int stateChange(int state, int card, int force)
{ 
  for (int slave = 1; slave <= NUMBER_OF_SLAVES; slave++) {
      checkTag(tag, tagarray, slave, false);
  }
  
  if (force >= 0) {
    state = force;
    if (state == 0) {
      myDFPlayer.playFolder(1, 100);
    } else {
    myDFPlayer.playFolder(1, state);
    }
    return state;
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

void revealColor() {
  colorWipe(pixels.Color(255, 0, 0), REVEAL_WAIT);
  colorWipe(pixels.Color(0, 0, 0), REVEAL_WAIT);
  colorWipe(pixels.Color(0, 255, 0), REVEAL_WAIT);
  colorWipe(pixels.Color(0, 0, 0), REVEAL_WAIT);
  colorWipe(pixels.Color(255, 0, 0), REVEAL_WAIT);
  colorWipe(pixels.Color(0, 0, 0), REVEAL_WAIT);
  colorWipe(pixels.Color(0, 0, 255), REVEAL_WAIT);
  colorWipe(pixels.Color(0, 0, 0), REVEAL_WAIT);
}
