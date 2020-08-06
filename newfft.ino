/*
 * Author: Priyath Fonseka
 * Date: August 2020
 *            RISING EDGE ELECTRONICS
 * 
 * Free to use/modify/distribute by public 
 * 
 * 
 * 
 */
#include "arduinoFFT.h"
#include <Adafruit_NeoPixel.h>

#define SAMPLES 64             //Must be a power of 2
#define SAMPLING_FREQUENCY 2000 //Hz, must be less than 10000 due to ADC
#define w 18
#define h 13
#define ledPIN 8
#define button 10
#define brightness 30

arduinoFFT FFT = arduinoFFT();
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(234, ledPIN, NEO_GRB + NEO_KHZ800);

unsigned int sampling_period_us;
unsigned long microseconds;

int state = LOW;
int pattern = 1; //pattern number incremented by the button 


double vReal[SAMPLES];
double vImag[SAMPLES];
byte data[SAMPLES/4];
byte freq[SAMPLES/4];

byte matrix[h][w];


void setup() {
    Serial.begin(115200);
    pinMode(A0,INPUT);
//pinMode(button,INPUT);
pinMode(ledPIN,OUTPUT);
    populateMatrix();
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
}
 
void loop() {
   
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = analogRead(A0);
        vImag[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
 
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);


//store the sampled signal amplitude and frequency into arrays for later use 
    for(int i=0; i<(SAMPLES/4); i++)
    {
        /*View all these three lines in serial terminal to see which frequencies has which amplitudes*/
         
        //Serial.print((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES, 1);
        //Serial.print(" ");
        Serial.println(vReal[i], 1);    //View only this line in serial plotter to visualize the bins

int f= (int) ((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES);
int col = map(f, 0 ,500, 0, w-1);
col = constrain(col, 0, w);

int amplitude= (int)vReal[i];
int height=map(amplitude,0,500,0,h-1);
height = constrain(height, 0, h);

data[i]=height;
freq[i]=col;       
    }

//debounce button
  if(debounceButton(state) == HIGH && state == LOW)
  {
    pattern++;
    if(pattern==8)
    pattern=1;
    
    state = HIGH;
  }
  else if(debounceButton(state) == LOW && state == HIGH)
  {
       state = LOW;
  }

//add custom pattern functions here 
switch(pattern)
{
  case 1:
  rgbHisto(255,0,0);
  break;
  case 2:
  rgbHisto(0,0,255);
  break;
        case 3:
  rgbHisto(0,255,255);
  break;
    case 4:
  rgbHisto(255,255,255);
  break;

  case 5:
  blueThree();
  break;
    case 6:
  colorWheel();
  break;
      case 7:
  triColor();
  break;
}
}



void rgbHisto(int r, int g, int b)
{
for(int i=0;i<SAMPLES/4;i++)
{

for(int j=h-1;j>=h-data[i];j--)
pixels.setPixelColor(matrix[j][freq[i]],pixels.Color(r,g,b));
}

  pixels.setBrightness(brightness);
pixels.show();

delay(10);
pixels.clear();
delay(10);
}

void blueThree()
{
for(int i=0;i<SAMPLES/4;i++)
{
for(int j=h-1;j>=h-data[i];j=j-3)
{
pixels.setPixelColor(matrix[j][freq[i]],pixels.Color(0,0,255));
pixels.setPixelColor(matrix[j-1][freq[i]],pixels.Color(0,0,255));
pixels.setPixelColor(matrix[j-2][freq[i]],pixels.Color(0,0,255));
} 
}
pixels.setBrightness(brightness);
pixels.show();
delay(10);
pixels.clear();
delay(10);
}

void colorWheel()
{
for(int i=0;i<SAMPLES/4;i++)
{
int wl=map(data[i],0,h-1,0,255);
for(int j=h-1;j>=h-data[i];j=j-3)
{
pixels.setPixelColor(matrix[j][freq[i]],Wheel(wl));
pixels.setPixelColor(matrix[j-1][freq[i]],Wheel(wl));
pixels.setPixelColor(matrix[j-2][freq[i]],Wheel(wl));
}  
}
 pixels.setBrightness(brightness);
pixels.show();
delay(10);
pixels.clear();
delay(10); 
}


void triColor()
{
for(int i=0;i<SAMPLES/4;i++)
{
for(int j=h-1;j>=h-data[i];j--)
{
  if(j>= (int)((h*2)/3))
pixels.setPixelColor(matrix[j][freq[i]],pixels.Color(0,255,0));
 if(j>= (int)(h/3) && j < (int)((h*2)/3))
 pixels.setPixelColor(matrix[j][freq[i]],pixels.Color(255,255,0));
 if(j< (int)(h/3))
  pixels.setPixelColor(matrix[j][freq[i]],pixels.Color(255,0,0));
}
}
  pixels.setBrightness(brightness);
pixels.show();
delay(10);
//off
pixels.clear();
delay(10); 
}

//populate the matrix grid with LED numbers corresponding to the hardware wiring 
void populateMatrix()
{
byte v=0;
int c=0;
for(byte i=0;i<h;i++)
{
for(byte j=0;j<w;j++)
{
if(i%2==0)
{
matrix[i][j]=c;
}
else
{
if(j==0)
{
  v=c+w-1;
}
else
{
  v--;
}
matrix[i][j]=v;
}
  c++;
}
} 
}

//////////////debouncing

boolean debounceButton(boolean state)
{
  boolean stateNow = digitalRead(button);
  if(state!=stateNow)
  {
    delay(10);
    stateNow = digitalRead(button);
  }
  return stateNow;
}

/* Utility from Adafruit Neopixel demo sketch
   Input a value 0 to 255 to get a color value.
   The colours are a transition R - G - B - back to R.*/
unsigned long Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
