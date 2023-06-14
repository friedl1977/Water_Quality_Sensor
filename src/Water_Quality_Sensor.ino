/*
 * Project Water_Quality_Sensor
 * Description:  Provdides Elicatrical Conducticity, Turbidity and TDS readings
 * Author:  F BASSON ~ FireFli
 * Date: 13 June 2023
 */

// Include ST7789 TFT Display libraries //
#include "../lib/Adafruit_GFX_RK/src/Adafruit_GFX.h"
#include "../lib/Adafruit_ST7735_RK/src/Adafruit_ST7789.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSansBold12pt7b.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSansBold9pt7b.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSans12pt7b.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSans9pt7b.h"
#include <SPI.h>

#define TurbiditySensorPin A4               // Sensor pin for the Turbidity Sensor
#define TdsSensorPin A3                     // Sensor pin for the TDS and EC Sensor
#define VREF 3.3                            // analog reference voltage of the ADC
#define SCOUNT 30                           // sum of sample point

// EC and TDS sensor declarations // 

int analogBuffer[SCOUNT];                   // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];               // DO NOT CHANGE THESE
int analogBufferIndex = 0;
int copyIndex = 0;

float prev_tdsValue = 0;
float tdsValue = 0;

float prev_ecValue = 0;
float ecValue = 0;

int temperature = 25;                       // Estimated water temperature.  Ideally we need to connect a temp sensor.
float averageVoltage = 0;

// Trubidity sensor declarations // 

int TurbiditySensorValue = 0;               // Raw value read by Turbudity Senspor
float turbidity_voltage = 0;                // Raw value mapped to 3V3
float volt = 0;
int sensorValue = 0;

float prev_turbidity = 0;
float turbidity = 0;


//float turbidity_percentage = 0; 

// ST7789 TFT  definitions // 
#define TFT_CS        S3                                            // Define CS pin for TFT display
#define TFT_RST       D6                                            // Define RST pin for TFT display
#define TFT_DC        D5                                            // Define DC pin for TFT display

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);     // Hardware SPI


void setup() {
  
 Serial.begin(115200);

  tft.init(320, 240);                                                 // Init ST7789 320x240 
  tft.fillScreen(ST77XX_BLACK);                                       // creates black background in display
  tft.setRotation(1); 

  draw_screen();
 
 pinMode(TurbiditySensorPin,INPUT);         // Declare analog pins as input pins
 pinMode(TdsSensorPin, INPUT);

}

void draw_screen() {

  tft.fillRect(0,0,155,115,ST77XX_BLUE);                                               // draws background fills for readings
  tft.fillRect(165,0,155,115,ST77XX_GREEN);                                            
  tft.fillRect(0,125,320,115,ST77XX_RED);

  ////// Main headings ///////
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextSize(1);
  tft.setTextWrap(false);

  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(10, 30);                                                                 // set sursor to start writing text
  tft.print("EC:  ");
  tft.setFont(&FreeSans9pt7b);
  tft.print("(mS/m)");

  tft.setFont(&FreeSansBold12pt7b);
  tft.setCursor(170, 30);
  tft.print("TDS:  ");
  tft.setFont(&FreeSans9pt7b);
  tft.print("(ppm/m)");

  tft.setFont(&FreeSansBold12pt7b);
  tft.setCursor(10, 150); 
  tft.print("Turbidity:  ");
  tft.setFont(&FreeSans9pt7b);
  tft.print("(NTU)");
}

void TDS() {

  static unsigned long analogSampleTimepoint = millis();

  if (millis()-analogSampleTimepoint > 40U) {                                                   //every 40 milliseconds,read the analog value from the ADC

    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);                                 //read the analog value and store into the buffer
    analogBufferIndex++;

      if (analogBufferIndex == SCOUNT)
          analogBufferIndex = 0;
  } 

  static unsigned long printTimepoint = millis();

  if(millis()-printTimepoint > 800U) {

    printTimepoint = millis();

    for(copyIndex = 0; copyIndex < SCOUNT; copyIndex ++)
        analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
        averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF/ 4095.0;           // read the analog value more stable by the median filtering algorithm, and convert to voltage value
        float compensationCoefficient = 1.0 + 0.02 * (temperature-25.0);                        // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
        float compensationVolatge = averageVoltage/compensationCoefficient;                     // temperature compensation

  tdsValue = (133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
  ecValue = tdsValue/6.41;

  Serial.print("TDS Value:");
  Serial.print(tdsValue, 2);
  Serial.println("ppm");

  Serial.print("EC Value:");
  Serial.print(ecValue, 2);
  Serial.println("mS/m");

  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextSize(2);
  tft.setTextWrap(false);
  
  tft.setCursor(15, 80);
  tft.setTextColor(ST77XX_BLUE);
  tft.println(prev_ecValue);
  tft.setCursor(15, 80);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(ecValue);

  prev_ecValue = ecValue;
  
  tft.setCursor(175, 80); 
  tft.setTextColor(ST77XX_GREEN);
  tft.println(prev_tdsValue);
  tft.setCursor(175, 80); 
  tft.setTextColor(ST77XX_WHITE);
  tft.println(tdsValue);
  
  prev_tdsValue = tdsValue;

  delay(50);
  
  //Particle.publish("TDS:" + String(tdsValue, 2) + "ppm", PRIVATE);        // Uncomment if you want to publish to Particle Cloud
  //Particle.publish("EC:" + String(ecValue, 2) + "mS/m", PRIVATE);

  }
}

void Turbidity() {
  
  for(int i=0; i<1000; i++) {                                               // Take 1000 samples and average 
    sensorValue += (analogRead(TurbiditySensorPin));
  }

    sensorValue = sensorValue/1000;

  //sensorValue = analogRead(TurbiditySensorPin);
  Serial.println(sensorValue);
  
  float turbidity = map(sensorValue, 1100, 1625, 100, 0);                   // sensor calibration, sensor min value, sensor max value.  Map to 0 - 100. 
  delay(100);

    if (turbidity < 0) { 
      turbidity = 0;
    } 
  
    if (turbidity < 20) {                                                   // You can determine you own thresholds.
        //Particle.publish("CLEAN: " + String(turbidity), PRIVATE);         // You can also add more if need be e.g. 0-5  = VERY CLEAN, 5-10 = CLEAN etc. 
          Serial.print("CLEAN: ");
          Serial.println(turbidity);
          tft.fillRect(115,151,180,59,ST77XX_RED);
          tft.setTextColor(ST77XX_WHITE);
          tft.setCursor(150, 205);
          tft.print("CLEAN");


        } else if ((turbidity > 20) && (turbidity < 50)) {                 // You can determine you own thresholds.
            //Particle.publish("MURKY: "+ String(turbidity), PRIVATE);
            Serial.print("MURKY: ");
            Serial.println(turbidity);
            tft.fillRect(115,151,180,59,ST77XX_RED);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(150, 205);
            tft.print("MURKY");
 
        
        } else if (turbidity > 50) {                                        // You can determine you own thresholds.
            //Particle.publish("DIRTY: "+ String(turbidity), PRIVATE);
            Serial.print("DIRTY: ");
            Serial.println(turbidity);
            tft.fillRect(115,151,180,59,ST77XX_RED);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(150, 205);
            tft.print("DIRTY");
        }

  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextSize(2);
  tft.setTextWrap(false);

  tft.setCursor(15, 205);
  tft.setTextColor(ST77XX_RED);
  tft.println(prev_turbidity);
  tft.setCursor(15, 205);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(turbidity); 

  prev_turbidity = turbidity;

  delay(50);
}

void loop() {
  
  TDS();                         // If using Particle Publish using timers not to exceed 
  Turbidity();                   // the rate limit!  DO NOT USE DELAY() as it will cause
                                 // the EC sensor to function incorrectly.                                                                                                     
}


  int getMedianNum(int bArray[], int iFilterLen) {                // median filtering algorithm - DO NOT CHANGE
    int bTab[iFilterLen];
    for (byte i = 0; i<iFilterLen; i++)
    bTab[i] = bArray[i];

  int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++) {
      for (i = 0; i < iFilterLen - j - 1; i++) {

  if (bTab[i] > bTab[i + 1]) {
    bTemp = bTab[i];
    bTab[i] = bTab[i + 1];
    bTab[i + 1] = bTemp;
        }  
      }
    }

  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}
