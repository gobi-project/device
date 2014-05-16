
// I2C to DMX on Arduino Uno
// Author: Fritz Jacob
// GOBI Master Project, TZI, University of Bremen

#include <Wire.h>
#include <DmxSimple.h>

#define RGB_ADDR 0x04
#define RGB_IDLE 0xF1
#define RGB_TEST 0xF2
#define RGB_TRAN 0xF3

#define LED_1 5
#define LED_2 3

byte state = RGB_IDLE;

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
  switch( state )
  {
    case RGB_IDLE: mode_idle(); break;
    case RGB_TRAN: mode_transmit(); break;
    case RGB_TEST: mode_test(); break;
    default: mode_idle(); break;    
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int num_bytes)
{
  digitalWrite(LED_1, HIGH);
  
  Serial.print("  data received, #bytes: ");
  Serial.print(num_bytes);
  Serial.println("");
  
  //if we receive only one byte we have state change
  if( num_bytes == 1 )
  {
    byte new_state = Wire.read();
    
    switch( new_state )
    {
      case RGB_IDLE: state = RGB_IDLE; break;
      case RGB_TEST: state = RGB_TEST; break;
      case RGB_TRAN: state = RGB_TRAN; test_done = false; break;
      default: state = RGB_IDLE; break;  
    }
    
    digitalWrite(LED_1, LOW);
    return;
  }
  
  //if we receive more than 1 byte and we are in TRAN(SMIT) mode, we set new values for rgb
  if( num_bytes == 3 && state == RGB_TRAN )
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
  
  Serial.println("  request event");
  
  if( state == RGB_TRAN )
  {
    Serial.println("requested RGB values");
    //return the current rgb values
    Wire.write( red );
    Wire.write( green );
    Wire.write( blue );
  }
  
  if( state == RGB_TEST && test_done )
  {
    Serial.println("requested end of test");
    state = RGB_TRAN;
    Wire.write( state );
  }
  
  digitalWrite(LED_2, LOW);
}

void writeDmxValues()
{ 
  DmxSimple.write(2, red);
  DmxSimple.write(3, green);
  DmxSimple.write(4, blue);
}

void mode_idle()
{
  Serial.println("Idle-Mode");
  delay(1000);
}


void mode_test()
{
  if( !test_done )
  {
    Serial.println("Test-Mode");
    Serial.print("red");
    DmxSimple.write(2, 255);
    delay(1000);
    DmxSimple.write(2, 0);
    Serial.print(" green");
    DmxSimple.write(3, 255);
    delay(1000);
    DmxSimple.write(3, 0);
    Serial.print(" blue\n");
    DmxSimple.write(4, 255);
    delay(1000);
    DmxSimple.write(4, 0);
    delay(1000);
    
    test_done = true;
  }
}

void mode_transmit()
{
  Serial.println("Transmit-Mode");
  Serial.print("red: ");
  Serial.print(red);
  Serial.print(", green: ");
  Serial.print(green);
  Serial.print(", blue: ");
  Serial.print(blue);
  Serial.println("");
  delay(1000);
}

