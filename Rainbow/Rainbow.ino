
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            0

// How many NeoPixels are attached to the Arduino?
#define NUM_LEDS     1
//to set a baseline volume, so I dont hard code in the wrong value
int baseLine = 0;
int amplitude = 20;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

 int readings =50;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  Serial.begin(115200);

   strip.begin(); // This initializes the NeoPixel library.
   strip.show();
   //20 K to get a decently average reading
   for(int i = 0; i<20000; i++) {
    baseLine += (analogRead(A0) * 10);
    }
    baseLine = baseLine / 20000;
    Serial.println(baseLine);
  
}
 
// the loop function runs over and over again forever
void loop() {
rainbowCycle(5);


}

byte*  applyAmplification(byte* input, byte amplification){
  //THIS ONLY WORKS FOR THE WHEEL / RAINBOW FUNCTION THE INPUT NEEDS TO BE byte[3]
  // I need to pull out the individual bytes, and modify them based on the 
  //microphone readings after the noise has been cancelled

  byte byteAmp = amplification;
  Serial.println(byteAmp);
  for(int i=0; i < 3; i++){
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

byte getAmplitude(){
  int difference  = readMicrophone(20) - baseLine;
  if(difference < 0){
    difference = difference * -1;
    }
  //noise filtering
  if(difference > 5){
    return byte(map(difference,6,50,10,255));    
    }
    else {
      return 0;
      }
}

int readMicrophone(int readings){
  int sum = 0;
  for (int i=0; i<readings; i++){
    //my artificial amplification, can be tweaked later
    sum += (analogRead(A0) * 10);
    }
    sum = sum / readings;
    return sum;
  }

// neopixel functions BELOW

void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    
    
   
    for(i=0; i< NUM_LEDS; i++) {
      c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
     c = applyAmplification(c, getAmplitude());
      setPixel(i, (*c), (*(c+1)), (*(c+2)));
    }
    showStrip();
    delay(SpeedDelay);
  }
}
  
void showStrip() {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif
}

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}

byte * Wheel(byte WheelPos) {
  static byte c[3];
    
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }
  return c;
}

