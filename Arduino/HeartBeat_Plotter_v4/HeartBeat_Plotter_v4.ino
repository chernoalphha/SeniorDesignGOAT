/*
  Heart beat plotting!
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 20th, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  Shows the user's heart beat on Arduino's serial plotter

  Instructions:
  1) Load code onto Redboard
  2) Attach sensor to your finger with a rubber band (see below)
  3) Open Tools->'Serial Plotter'
  4) Make sure the drop down is set to 115200 baud
  5) Checkout the blips!
  6) Feel the pulse on your neck and watch it mimic the blips

  It is best to attach the sensor to your finger using a rubber band or other tightening
  device. Humans are generally bad at applying constant pressure to a thing. When you
  press your finger against the sensor it varies enough to cause the blood in your
  finger to flow differently which causes the sensor readings to go wonky.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
*/

#include <Wire.h>
#include "MAX30105.h"
#include <TimerOne.h>

MAX30105 particleSensor;
int samples;
int numInts = 0; //number of interrupts
int secTime = 15; 
volatile bool takeSamples = false;
volatile int state = 0; //0 = do nothing, 1 = take respiration measurements, 2 = take hydration measurements

void setup()
{

  samples =0;
  //takeSamples = false;
 
  Serial.begin(115200);
  Serial.setTimeout(300);
  //Serial.println("Initializing...");
  
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  //Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  //Arduino plotter auto-scales annoyingly. To get around this, pre-populate
  //the plotter with 500 of an average reading from the sensor

  //Take an average of IR readings at power up
  /*const byte avgAmount = 64;
  long baseValue = 0;
  for (byte x = 0 ; x < avgAmount ; x++)
  {
    baseValue += particleSensor.getIR(); //Read the IR value
  }
  baseValue /= avgAmount;*/
  //1000000 = 1 second
    Timer1.initialize(1000000);   //interrupt every 0.2 seconds


  //Pre-populate the plotter so that the Y scale is close to IR values
  /*for (int x = 0 ; x < 500 ; x++)
    Serial.println(baseValue); */
}

void sample(void)
{ 
    numInts++;
    Serial.println(numInts);
    //Serial.println(particleSensor.getIR()); //Send raw data to plotter
    
    //Serial.println(particleSensor.getIR()); //Send raw data to plotter
    if (state ==0 ||numInts >= secTime + 1){//stop interrupt after given amount of seconds
      //account for 2 interrupt delay by adding 2
      //takeSamples = false;
      state = 0;
      numInts = 0;
      Timer1.detachInterrupt();
    }

}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  if (Serial.available()){
    String msg;
    msg = Serial.readString();  
     //first letter indicates hydration ('h') or respiration ('r'). And message must be less than 5 characters
    if (msg.length() == 1 && msg.charAt(0) == 'c'){ //cancel measurements
      state = 0;
    }
    
    if (msg.length() < 5 && (msg.charAt(0) == 'h' || msg.charAt(0) == 'r')){
      Serial.println(msg);  //for debugging
   
      if (msg.charAt(0) == 'r'){ //take respiration measurements
        state = 1;
      }
      
      if (msg.charAt(0) == 'h'){ //take hydration measurements
        state = 2;
      }
      msg.remove(0,1);  //discard first letter that indicates hydration/respiration
      secTime = msg.toInt(); //set length of time for sampling
      Timer1.attachInterrupt(sample);
    }
  }
}

void loop()
{
  //Serial.println(state);
  if (state){
    if (state == 1){
      Serial.println(particleSensor.getIR()); //Send raw data
    }
    if (state == 2){
      Serial.println("hydr");
    }
  }
    /*if (Serial.available()){
      String msg = Serial.readString();
      
       //first letter indicates hydration ('h') or respiration ('r'). And message must be less than 5 characters
      if (msg.length() < 5 && msg.charAt(0) == 'h'){    
        //Serial.println("Start"); 
       
       
        Serial.println(secTime);
       
        takeSamples = true;
        while(takeSamples){
           //Serial.println(particleSensor.getIR()); //Send raw data
        }
      }
      if (msg.length() 
    }*/
}



