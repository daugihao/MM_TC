#include <Wire.h>

int x = 0;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  //Wire.onReceive(receiveEvent);
  Serial.begin(9600);  // start Serial for output
  Serial.println("Tarot master is ready to receive data!");
}

void loop()
{
  // Request data from first card reader
  Wire.requestFrom(1, 1); 
  int n1 = Wire.read();
  Serial.print("First card reader: ");
  Serial.println(n1);

  // Request data from second card reader
  Wire.requestFrom(2, 1); 
  int n2 = Wire.read();
  Serial.print("Second card reader: ");
  Serial.println(n2);

  delay(500);
}
