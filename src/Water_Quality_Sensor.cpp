/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/friedl/Desktop/Projects/Water_Quality_Sensor/src/Water_Quality_Sensor.ino"
/*
 * Project Water_Quality_Sensor
 * Description:
 * Author:
 * Date:
 */
 
#include "math.h" 
 
void setup();
void TDS();
void Turbidity();
void loop();
int getMedianNum(int bArray[], int iFilterLen);
#line 10 "/Users/friedl/Desktop/Projects/Water_Quality_Sensor/src/Water_Quality_Sensor.ino"
#define TurbiditySensorPin A0
#define TdsSensorPin A5
#define VREF 3.3                            // analog reference voltage(Volt) of the ADC
#define SCOUNT 30                           // sum of sample point

// EC and TDS sensor declarations // 

int analogBuffer[SCOUNT];                   // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float ecValue = 0;
int temperature = 25;

// Trubidity sensor declarations // 

int TurbiditySensorValue = 0;
float turbidity_voltage = 0;
float volt = 0;
float ntu = 0;
float turbidity_percentage = 0;


void setup() {
  
 Serial.begin(115200);
 pinMode(TurbiditySensorPin,INPUT);
 pinMode(TdsSensorPin, INPUT);

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
  
  //Particle.publish("TDS:" + String(tdsValue, 2) + "ppm", PRIVATE);
  //Particle.publish("EC:" + String(ecValue, 2) + "mS/m", PRIVATE);

  }
}

void Turbidity() {

  int sensorValue = 0;
  
  for(int i=0; i<1000; i++) {
    sensorValue += (analogRead(TurbiditySensorPin));
  }

    sensorValue = sensorValue/1000;

    
  //sensorValue = analogRead(TurbiditySensorPin);
  Serial.println(sensorValue);
  
  int turbidity = map(sensorValue, 30, 990, 100, 0);
  delay(100);
  
    if (turbidity < 20) {
        //Particle.publish("its CLEAR " + String(turbidity), PRIVATE);
  
        } else  if ((turbidity > 20) && (turbidity < 50)) {
            //Particle.publish("its CLoUDY "+ String(turbidity), PRIVATE);
        
        } else if (turbidity > 50) {
            //Particle.publish("its DiRTY "+ String(turbidity), PRIVATE);
        }

        Serial.println(turbidity);
}

void loop() {
  
  TDS();
  Turbidity();
  //turbidity_exp ();
  //delay(10000);

}


  int getMedianNum(int bArray[], int iFilterLen) {
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

// void turbidity_exp () {

//   volt = 0;

//     for(int i=0; i<1000; i++) {
        
//     volt += ((float)analogRead(TurbiditySensorPin)/4095)*3.3;
//     }
    
//     volt = volt/1000;
  
//         if (volt < 1.6913) {
//             ntu = 3000;
//         }
//             else if (volt < 2.7720) {
//                 ntu = -2572.2 * pow(volt, 2) + 8700.5 * volt - 4352.9;
//         }
//             else {
//                 ntu = 0;
//         }
  
//     turbidity_percentage = ((3.5/4550)*ntu);
//     Particle.publish("NTU: " + String(ntu,2) + "NTU Ratio: " + String(turbidity_percentage, 2), PRIVATE);
  
//     delay(100); 
    
//}
