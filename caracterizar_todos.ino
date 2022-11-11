  #include "SingleEMAFilterLib.h"
SingleEMAFilter<int> singleEMAFilter(0.3);

const int dedoAct= 1;
// Pines del sistema
const int valvulaPin[] = {8,11,12,9,10};
const int motorPin[] = {5,4,7,6,3};
const int flexPin[] = {A2, A1, A0, A3, A4};

const int buttonPin = 2;     // the number of the pushbutton pin

// Variables de escalon y boton de control
int buttonState = true;         // variable for reading the pushbutton status
int PWM_High = 180;
int PWM_Low = 0;

float resistencia[5];
float porcentaje[5];
float porcentajeFiltrado[5];

//Variables para escribir en csv
char dataStr[100] = "";
char buffer[7];

// Constantes del sensor
const float VCC = 5;      // Voltaje del arduino
const float R_DIV[] = {23000.0, 4600.0, 6000.0, 20000.0, 20200.0};  // Resistencia en divisor de voltaje
const float flatResistance[] = {23000.0, 4200.0, 6000.0, 15000.0, 29000.0}; // Resistencia minima (plano)
const float bendResistance[] = {34000.0, 1000.0, 1000.0, 22200.0, 42000.0};  // Resistencia maxima (doblado)

void setup() {
  
  Serial.begin(57600);
  
  // initialize the LED pin as an output:
  for (byte i = 0; i < 5; i = i + 1) {
    pinMode(valvulaPin[i], OUTPUT); 
    pinMode(motorPin[i], OUTPUT);
    digitalWrite(motorPin[i], HIGH);

    pinMode(flexPin[i], INPUT);
  }

  // initialize the pushbutton pin as an input:
  attachInterrupt(digitalPinToInterrupt(buttonPin), changeMode, RISING);
}

void loop() {
  
  for (byte i = 1; i < 2; i = i + 1) {
    resistencia[i] = resistenciaDef(i);
    porcentaje[i] = porcentajeDef(resistencia[i], i);
    singleEMAFilter.AddValue(porcentaje[i]);
    porcentajeFiltrado[i] = singleEMAFilter.GetLowPass();
  }
  
  dataStr[0] = 0;                //clean out string

  ltoa( millis(),buffer,10);     //convert long to charStr
  strcat(dataStr, buffer);       //add it to the end
  strcat( dataStr, ", "); //append the delimiter
  
  for (byte i = 1; i < 2; i = i + 1) {
    dtostrf(porcentaje[i], 5, 1, buffer);  //5 is minimum width, 1 is precision; float value is copied onto buff
    strcat( dataStr, buffer); //append the converted float
    strcat( dataStr, ", "); //append the delimiter
  
    dtostrf(porcentajeFiltrado[i], 5, 1, buffer);  //5 is minimum width, 1 is precision; float value is copied onto buff
    strcat( dataStr, buffer); //append the converted float
    strcat( dataStr, ", "); //append the delimiter

  }

  dtostrf(PWM_High, 5, 1, buffer);  //5 is minimum width, 1 is precision; float value is copied onto buff
  strcat( dataStr, buffer); //append the converted float
  strcat( dataStr, 0); //terminate correctly 

  if(!buttonState){
    Serial.println(dataStr);
  }
  else{
    digitalWrite(valvulaPin[dedoAct], LOW);    
  }

  delay(10);
}

void changeMode(){
  
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 300) 
  {
    if (buttonState) {
      // turn valve on:
      analogWrite(valvulaPin[dedoAct], PWM_High);
      //analogWrite(valvulaPin[0], PWM_High);
    } else {
      // turn valve off:
      digitalWrite(valvulaPin[dedoAct], LOW);    
      //digitalWrite(valvulaPin[0], LOW); 
    }
    buttonState = !buttonState;
  }
  last_interrupt_time = interrupt_time;
}
/*
float resistenciaDef(byte dedo){
  int ADCflex = analogRead(flexPin[dedo]);
  float Vflex = ADCflex * VCC / 1023.0;
  float Rflex = (Vflex * R_DIV[dedo]) / (VCC - Vflex);
  return Rflex;
}
*/

float resistenciaDef(byte dedo){
  int ADCflex = analogRead(flexPin[dedo]);
  float Vflex = ADCflex * VCC / 1023.0;
  float Rflex = (Vflex * R_DIV[dedo]) / (VCC - Vflex);
  return Rflex;
}
float porcentajeDef(float Rflex, byte dedo){
  float porcentaje = map(Rflex, flatResistance[dedo], bendResistance[dedo], 0, 100.0);
  return porcentaje;
}
