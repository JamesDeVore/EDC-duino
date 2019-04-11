#include <Adafruit_DotStar.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN 1

// How many NeoPixels are attached to the Arduino?
#define NUM_LEDS 2
//to set a baseline volume, so I dont hard code in the wrong value

//Microphone stuff
int baseLine = 0;
int amplitude = 20;
int adjustedBaseline = 0;

const int sensorPin = A4; // pin that the sensor is attached to
int sensorValue = 0;           // the sensor value
int sensorMin = 1023;          // minimum sensor value
int sensorMax = 0;             // maximum sensor value
int calibrationCheckArray[50]; //array to keep track of readings to check for auto calibration
byte arrayPointer = 0; 
int prevRead = 0; //to dynamically find differences
int readings = 50;

//Button stuff
const int buttonPin = 0; // the number of the pushbutton pin
int buttonState = 0;     // variable for reading the pushbutton status
int mode = 0;

//LED 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_DotStar onBoard = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);


//Blink without delay stuff
// the setup function runs once when you press reset or power the board
unsigned long previousMillis = 0; // will store last time LED was updated

// constants won't change:
const long interval = 2500; // interval at which to blink (milliseconds)


void setup()
{
  pinMode(buttonPin, INPUT);
  strip.begin(); // This initializes the NeoPixel library.
  onBoard.begin();
  onBoard.show();
  strip.show();
  setAll(255, 0, 0);
  delay(1000);
  arduinoCalibration();
  delay(3000);
  setAll(0, 0, 0);
}

// the loop function runs over and over again forever
void loop()
{

  /*
   if(millis() - previousMillis >= interval){
    previousMillis = millis();
  if(mode == 0){
    mode = 1;
    } else{
      mode = 0;
      }
  }
    
  if(mode == 1){
    showOneColor(255,0,0);
    }
   if(mode == 0){
    showOneColor(0,0,255);
    }

*/

  //Mikey
  //showOneColor(255,0,0);

  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW)
  {
    // turn LED on:
    setAll(0, 250, 0);
  }
  else
  {
    // turn LED off:
    rainbowCycle(10);
  }
}

// Calibration options
void arduinoCalibration()
{

  Serial.begin(115200);
  unsigned long now = millis();
  unsigned long readingCount = 0;
  // calibrate during the first five seconds
  while (now + 5000 > millis())
  {
    sensorValue = readPin(1);
    baseLine += sensorValue;
    readingCount++;
    //Serial.println(sensorValue);
    // record the maximum sensor value
    if (sensorValue > sensorMax)
    {
      sensorMax = sensorValue;
    }

    // record the minimum sensor value
    if (sensorValue < sensorMin)
    {
      sensorMin = sensorValue;
    }
  }
  baseLine = baseLine / readingCount;
  // signal the end of the calibration period

  Serial.println(sensorMin);
  Serial.println(sensorMax);
  Serial.println(baseLine);
}
int readPin(int multiplier)
{
  return analogRead(sensorPin) * multiplier;
}

byte *applyAmplification(byte *input, byte amplification)
{
  //THIS ONLY WORKS FOR THE WHEEL / RAINBOW FUNCTION THE INPUT NEEDS TO BE byte[3]
  // I need to pull out the individual bytes, and modify them based on the
  //microphone readings after the noise has been cancelled

  byte byteAmp = amplification;
  //Serial.println(byteAmp);
  for (int i = 0; i < 3; i++)
  {
    int temp = int(input[i]);
    //255 is the max value, I can turn this down if it filters it too much
    //But its based from my map in the amplitude function, max value won't affect this
    //byte
    temp = temp * byteAmp / 255;
    input[i] = byte(temp);
  }

  //Serial.println(input[0]);
  return input;
}

int newRead(int readings)
{
  //analogWrite(sensorPin,1);
  // sensorValue = analogRead(sensorPin);

  int sum = 0;
  for (int i = 0; i < readings; i++)
  {
    //my artificial amplification, can be tweaked later
    sum += readPin(1);
  }
  sum = sum / readings;
  //Serial.println(sum);

  // apply the calibration to the sensor reading
  //sensorValue = map(sensorValue, sensorMin, sensorMax, 0, 255);

  // in case the sensor value is outside the range seen during calibration
  //sensorValue = constrain(sensorValue, 0, 255);

  //Serial.println(sum);
  return sum;
}

byte newAmplitude()
{
  Serial.println(newRead(100));

  float difference = (newRead(100) - baseLine);
  if (difference < 0)
  {
    difference = difference * -1;
  }
  /*calibrationCheckArray[arrayPointer] = difference;
  arrayPointer++;
  if(arrayPointer == 50){
    arrayPointer = 0;
    }
    
    
   
    int calSum = 0;
   for (int i = 0; i< 50; i++){
    calSum += calibrationCheckArray[i];
    }
    calSum = calSum / 50;
    //Serial.println(calSum); 
    if(calSum > 30){
      memset(calibrationCheckArray, 0, sizeof(calibrationCheckArray));
      //setup();
      }
      */
  //Serial.println(difference);

  if (prevRead - difference > 0)
  {
    prevRead = difference;
    return byte(map(difference, 10, 50, 10, 255));
  }
  else
  {
    prevRead = difference;
    return 10;
  }
}
// neopixel functions BELOW

void showOneColor(byte red, byte green, byte blue)
{
  float factor = float(newAmplitude()) / 255;
  //Serial.println(factor);
  float newRed = byte(float(red) * factor);
  float newGreen = byte(float(green) * factor);
  float newBlue = byte(float(blue) * factor);
  if (newRed < 20)
  {
    newRed = 0;
  }
  if (newBlue < 20)
  {
    newBlue = 0;
  }
  if (newGreen < 20)
  {
    newGreen = 0;
  }
  setAll(newRed, newGreen, newBlue);
}

void rainbowCycle(int SpeedDelay)
{
  byte *c;
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++)
  { // 5 cycles of all colors on wheel

    for (i = 0; i < NUM_LEDS; i++)
    {
      c = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      c = applyAmplification(c, newAmplitude());
      setPixel(i, (*c), (*(c + 1)), (*(c + 2)));
      delay(10);
    }
    showStrip();
    delay(SpeedDelay);
  }
}

void showStrip()
{
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.show();
  onBoard.show();
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  FastLED.show();
#endif
}

void setPixel(int Pixel, byte red, byte green, byte blue)
{
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.setPixelColor(Pixel, strip.Color(red, green, blue));
  onBoard.setPixelColor(Pixel, strip.Color(red, green, blue));
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
#endif
}

void setAll(byte red, byte green, byte blue)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    setPixel(i, red, green, blue);
  }
  showStrip();
}

byte *Wheel(byte WheelPos)
{
  static byte c[3];

  if (WheelPos < 85)
  {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  }
  else
  {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }
  return c;
}

