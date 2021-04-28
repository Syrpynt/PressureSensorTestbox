
/*
   Project: Suspension Control
   Purpose: Multiplexed I2C pressure sensor testbox
   Created On: 8/6/2019
   Last Edited By: Michael Cohen
   Last Edit Date: 8/16/2019
   Created By: Michael Cohen
   Reviewed By: Levi Danzer
*/

//add necessary communication libraries
#include <SoftwareSerial.h>
#include <Wire.h>

#define ROWS 2
#define COLS 16

uint8_t maxSensors = 6; //Max Number of Sensors Reading

#define TCAADDR 112 //I2C 0x70 address of the I2C mux
#define PRESSUREADDR 0x28 //Address of pressure sensor
#define CONVERSFACTR 0.009155832265152 //calibration value for a 150psi (150/0x3fff)

#define TxPin 6 //LCD Transmission Pin
//ParallaxLCD LCD = ParallaxLCD(TxPin, ROWS, COLS); //Pin 6 will output to LCD, Rows, Column
SoftwareSerial Tx = SoftwareSerial(255, TxPin);

uint16_t volatile sensorData = 0;
uint8_t volatile firstByte = 0;
uint8_t volatile secondByte = 0;
// float dataToDisplay = 0;
// uint8_t previousButtonState = 0;

const int buttonPin = 5; /*Button is connected to pin 5 because analog
                          pins don't have pull-up resistors*/
int buttonState = 0;
int lastButtonState = 1;
uint8_t lastBounceTime = 0; //The last time the output pin was toggled
long bounceDelay = 350; //increase if output flickers
uint8_t sensorNumber = 1;
uint8_t sensorLimit = 7;

void tcaSelect(uint8_t i2cp)
{
  Tx.print(i2cp); //A) tca select output
  Tx.print(" "); //A) tca select output
  Wire.beginTransmission(TCAADDR);
  uint8_t hobo = 1 << i2cp; //A) tca select output
  Wire.write(hobo); //A) tca select output
  Tx.print(hobo); //A) tca select output
  Wire.endTransmission(true);
}

uint16_t getSensorData()
{
  uint8_t volatile firstByte = 0;
  uint8_t volatile secondByte = 0;
  Wire.requestFrom(PRESSUREADDR, 2, true); // request 2 bytes from slave device
  delay(10); //Wait for the data to show up
  while ( Wire.available()) // slave may send less than requested
  {
    firstByte = Wire.read();
    delay(10);
    secondByte = Wire.read();
    delay(10);
  }
  //trim the first byte of the status and combine to make the data
  sensorData = ((firstByte & 0x3f) << 8) + secondByte;
  return sensorData;
}



//Initial setup
void setup()
{
  Tx.begin(19200); //Initiate LCD serial
  delay(100); //Necessary delay
  Tx.write(12); //Clears screen
  Tx.write(17); //backlight ON
  Tx.write(22); //Cursor OFF
  delay(10);

  pinMode(buttonPin, INPUT_PULLUP); //Initiate internal pull-up resistor and button

  Wire.begin(); //Initiate I2C

  Tx.print("Setup Complete"); //Screen comment on completion of startup.
  delay(1500);

  Tx.write(12); //clear
  Tx.print("Press button to");
  Tx.write(13); //next line
  Tx.print("cycle sensors");
  delay(2000);
  Tx.write(12); //A) tca select output
  tcaSelect(sensorNumber - 1);
}

//Main Program
void loop()
{
  delay(50);
  lastButtonState = buttonState;
  buttonState = digitalRead(buttonPin);
  lastBounceTime = bounceDelay - millis();
  if ( (millis() - lastBounceTime) > bounceDelay)
  {
    if ( (0 == buttonState) && (1 == lastButtonState) )
    {
      Tx.write(12); //A) tca select output
      sensorNumber++;
      if (sensorNumber >= sensorLimit )
      {
        sensorNumber = 1;
      }
      tcaSelect(sensorNumber - 1);
    }
  }
  float pressure = CONVERSFACTR * getSensorData();
  
  Tx.write(12); //clear before printing new sensor and data
  Tx.print("Sensor ");
  Tx.print(sensorNumber);
  Tx.print(" Signal");
  Tx.write(13); //next line
  Tx.print(pressure, 2);
  Tx.print(" psi  ");
  Tx.print(getSensorData());
  delay(150);

  //Tx.write(12);
  //Tx.write(buttonState); //Uncomment to see "\" for closed and "~" for open button state
}
