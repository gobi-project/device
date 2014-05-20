
// I2C to DMX on Arduino Uno
// Author: Fritz Jacob
// GOBI Master Project, TZI, University of Bremen

#include <Wire.h>
#include <DmxSimple.h>

#define RGB_ADDR 4

#define LED_1 5
#define LED_2 3

byte red = 0;
byte green = 0;
byte blue = 0;

boolean test_done = false;

void setup()
{
  DmxSimple.usePin(6);
  DmxSimple.maxChannel(10);
  
  Wire.begin( RGB_ADDR );                // join i2c bus with address #4
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  pinMode(LED_1, OUTPUT);  
  pinMode(LED_2, OUTPUT);  
  
  Serial.begin(9600);
}

void loop()
{
  delay(1000);
  run_test();  //runs one time!
  Serial.println("Idle");
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int num_bytes)
{
  digitalWrite(LED_1, HIGH);
  
  Serial.print("data received, #bytes: ");
  Serial.print(num_bytes);
  Serial.println("");
  
  if( num_bytes == 3 )
  {
    if( Wire.available() )
    {
      red = Wire.read();
    }
    
    if( Wire.available() )
    {
      green = Wire.read();
    }
    
    if( Wire.available() )
    {
      blue = Wire.read();
    }
    
    writeDmxValues();
  }
  
  digitalWrite(LED_1, LOW);
}

void requestEvent()
{
  digitalWrite(LED_2, HIGH);

  Serial.println("requested RGB values");
  Serial.print( red );
  Serial.print( green );
  Serial.print( blue );
  Serial.print( "\n" );
  //return the current rgb values
  byte rgb[3];
  rgb[0] = red;
  rgb[1] = green;
  rgb[2] = blue;
  
  Wire.write( rgb, 3 );
  
  digitalWrite(LED_2, LOW);
}

void writeDmxValues()
{ 
  DmxSimple.write(2, red);
  DmxSimple.write(3, green);
  DmxSimple.write(4, blue);
}

void run_test()
{
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, HIGH);
  
  if( !test_done )
  {
    Serial.println("Test");
    Serial.print("red");
    DmxSimple.write(2, 255);
    delay(500);
    DmxSimple.write(2, 0);
    Serial.print(" green");
    DmxSimple.write(3, 255);
    delay(500);
    DmxSimple.write(3, 0);
    Serial.print(" blue\n");
    DmxSimple.write(4, 255);
    delay(500);
    DmxSimple.write(4, 0);
    delay(500);
    
    test_done = true;
  }
  
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
}

