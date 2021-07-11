
#include "Arduino.h"
#include "SoftwareSerial.h"


// slowwire START ***********************************************************************************************

// global variables

#define slowwire_TIMEOUT      120   // do not change
#define slowwire_MAX_SENSORS   2    // Max number of Slowwire sensors conected

uint8_t sw_data[5];
uint8_t sw_sensor_num = 0;

struct slowwireSTRUCT {
  long humidity = 0;
  long bits = 0;
  uint16_t pin = 0;
  String nameID;
} slowwire[slowwire_MAX_SENSORS];


void slowwireInit(void)
{

// SLOWWIRE SETTING HERE ----------------------------------------------------------------------------------------
sw_sensor_num=2;  // should be lower or equal to the slowwire_MAX_SENSORS defined above

slowwire[0].pin=3;
slowwire[0].nameID="primo"; // max 9 characters

slowwire[1].pin=4;
slowwire[1].nameID="secondo"; // max 9 characters


  
  uint8_t sensorinde=0;
  for (sensorinde=0 ; sensorinde<sw_sensor_num; sensorinde++) {

    Serial.println("NemaID: "+ slowwire[sensorinde].nameID + " Pin: " + String(slowwire[sensorinde].pin));
    
  }
}



bool slowwireWaitState(uint8_t level, uint8_t sensorinde)
{
  unsigned long timeout = micros() + 50000;
  while (digitalRead(slowwire[sensorinde].pin) != level) {

    if (micros() > timeout) {
      return false;
    }
    delayMicroseconds(50);
  }
  return true;
}

static uint8_t Compute_CRC8_Simple(uint8_t* byte,int numbytes)  // generates 8 bit CRC from a given array of bytes
{
    const uint8_t generator = 0x1D;
    uint8_t crc = 0; /* start with 0 so first byte can be 'xored' in */
	uint8_t currByte =0 ;
    for (int i=0; i<numbytes; i++) 
    {
		currByte=byte[i];
        crc ^= currByte; /* XOR-in the next input byte */

        for (int i = 0; i < 8; i++)
        {
            if ((crc & 0x80) != 0)
            {
                crc = (uint8_t)((crc << 1) ^ generator);
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}


bool slowwireRead(uint8_t sensorinde)
{

  slowwire[sensorinde].bits = 0;
  sw_data[0] = sw_data[1] = sw_data[2] = sw_data[3] = sw_data[4] = 0;
  slowwire[sensorinde].humidity = -1;

  uint32_t numcycles=100;
  uint8_t count_ext=0;
  uint32_t count = numcycles;
  while ((count_ext<3)&&(count>=numcycles)) {

    pinMode(slowwire[sensorinde].pin, OUTPUT);

    delay(100);
    digitalWrite(slowwire[sensorinde].pin, HIGH);

    delay(5);

    digitalWrite(slowwire[sensorinde].pin, LOW);

    delay(19);  // minimum 18ms

    digitalWrite(slowwire[sensorinde].pin, HIGH);

    delay(10);
    pinMode(slowwire[sensorinde].pin, INPUT);
    digitalWrite(slowwire[sensorinde].pin, HIGH);       // turn on pullup resistors

 // Wait for sensor to pull pin low.
    count = 0;
    while ((digitalRead(slowwire[sensorinde].pin) != 0)&&(count<numcycles)) {
      count++;
      delay(3);
    }
    //Serial.println(String(count) + " PIN " + String(slowwire[sensorinde].pin));
    count_ext++;

  }
  if (count>=numcycles)
  {  
      return false; 
  }

  // for debug check if the system gets answer
  //slowwire[sensorinde].humidity=count;
  // pin is now zero
  //noInterrupts();
  // signal is made in the way that one bit is transmitted with a low pulse and a high pulse
  // when 1 the high pulse is 1/4 the lenght of the low pulse, 
  // when 0 the high pulse is the same lenght of the low pulse
  // low pulse lenght is constant to 10ms
  //noInterrupts();

  int i;

    for (i = -1; i < 40; i++) {
      if (!slowwireWaitState(1,sensorinde)) { break; }
      delay(6);                          // 8 seem to be OK
      
      if (i>=0){
        if (digitalRead(slowwire[sensorinde].pin)==0) {           
        //sw_data[i / 8] |= (1 << (7 - i % 8));        
        sw_data[i / 8] |= (1 << (i % 8));        
        }
      }
      if (!slowwireWaitState(0,sensorinde)) { break; }
    }

  pinMode(slowwire[sensorinde].pin, INPUT);

  //digitalWrite(slowwire[sensorinde].pin, HIGH);

  //interrupts();
  slowwire[sensorinde].bits = i;
  if (i < 16) { return false; }
 
  
  if ((Compute_CRC8_Simple(sw_data,2)-sw_data[2])==0) {
    slowwire[sensorinde].humidity = ((sw_data[1] << 8) | sw_data[0]);
    return true;
  }



  return false;
}





// SLOWWIRE END ****************************************************************************************************




 
// Configuration 
// -------------
void setup() {
 
  
  Serial.begin(9600);
  Serial.setTimeout(20);
 
  // Slowwire initialization

  slowwireInit();

}
 
void loop() {
 
  String timestamp="1111";

  uint8_t sensorinde=0;
  // real all the data from the Sensors
  for (sensorinde=0 ; sensorinde<sw_sensor_num; sensorinde++) {
    bool isok = slowwireRead(sensorinde);
    delay(1000);
   
  }


  for (sensorinde=0 ; sensorinde<sw_sensor_num; sensorinde++) {
      // Message made as nameID:timestamp:datastr
      String message;
      message=slowwire[sensorinde].nameID + ":" + String(slowwire[sensorinde].humidity);
      Serial.println(message);
      delay(2000);
   }


  // wait at least 2000 ms
  delay(1000);
 

}