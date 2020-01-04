#include <Wire.h>

byte tagarray[][4] = {
  {0x5C, 0xF9, 0x9D, 0xD6},
  {0xCC, 0x21, 0x9D, 0xD6}, 
  {0xDC, 0xCB, 0x9C, 0xD6},
  {0xEC, 0xC5, 0x9C, 0xD6},
  {0xCC, 0x1D, 0x9D, 0xD6},
};

volatile byte tagreading[][4] = {0xFF, 0xFF, 0xFF, 0xFF};

int x = 0;
int x_master = 0;
int data_length = 4;

void setup()
{
  Wire.begin(0);        // join i2c bus (address optional for master)
  //Wire.onReceive(receiveEvent);
  Serial.begin(9600);  // start Serial for output
  Serial.println("Tarot master is ready to receive data!");
}

void loop()
{
  // Request data from first card reader
  Wire.requestFrom(1, data_length);
  for (int i = 0; i < data_length; i++) {
    tagreading[1][i] = Wire.read();
  }
  Serial.print("First card reader: ");
  Serial.print(tagreading[1][0]);
  Serial.print(tagreading[1][1]);
  Serial.print(tagreading[1][2]);
  Serial.println(tagreading[1][3]);

  delay(1000);

  // Request data from second card reader
  Wire.requestFrom(2, data_length);
  for (int i = 0; i < data_length; i++) {
    tagreading[1][i] = Wire.read();
  }
  Serial.print("Second card reader: ");
  Serial.print(tagreading[1][0], HEX);
  Serial.print(tagreading[1][1], HEX);
  Serial.print(tagreading[1][2], HEX);
  Serial.println(tagreading[1][3], HEX);

  delay(1000);
}
