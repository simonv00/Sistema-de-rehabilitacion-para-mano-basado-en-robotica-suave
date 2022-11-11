const int valvulaPin[] = {8,11,12,9,10};
const int motorPin[] = {5,4,7,6,3};
const int flexPin[] = {A2, A1, A0, A3, A4};

const int q0[] = {20, 10, 20, 7.8, 20};
const int q1[] = {-20, -10, -20, -7.3, -20};

int e_1[] = {0, 0, 0, 0, 0};
int u_1[] = {0, 0, 0, 0, 0};
float uMod[5];
float u[5];

int u_1_m[] = {0, 0, 0, 0, 0};
float uMod_m[5];
float u_m[5];
 
int porcetajeDeseado[] = {0, 0, 0, 0, 0};

float resistencia[5];
float porcentaje[5];

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];  
int accion = 0;
boolean newData = false;

const int maxPWM = 255;

// Constantes del sensor
const float VCC = 5;      // Voltaje del arduino
const float R_DIV[] = {23000.0, 4600.0, 6000.0, 20000.0, 20200.0};  // Resistencia en divisor de voltaje
const float flatResistance[] = {28500.0, 2800.0, 6000.0, 17000.0, 29000.0}; // Resistencia minima (plano)
const float bendResistance[] = {38000.0, 1000.0, 1000.0, 24200.0, 40000.0};  // Resistencia maxima (doblado)

unsigned long previousMillis = 0;        // will store last time LED was updated

void setup() {
  Serial.begin(115200);
  for (byte i = 0; i < 5; i = i + 1) {
    pinMode(valvulaPin[i], OUTPUT); 
    pinMode(motorPin[i], OUTPUT);
    digitalWrite(motorPin[i],HIGH);
    pinMode(flexPin[i], INPUT);
  }
}

void Stop(){
  for (byte i = 0; i < 5; i = i + 1) {
    digitalWrite(valvulaPin[i], LOW); 
    digitalWrite(motorPin[i], LOW);
  }
}

float PWM(int control){
  if(control > maxPWM){
    control = maxPWM;
  }
  else if(control < 0){
    control = 0;
  }
  return control;
}

void loop() {

  
  int porcetajeActual[5];
  float e[5];

  for(int i = 0; i < 5; i++){
    resistencia[i] = resistenciaDef(i);
    porcentaje[i] = porcentajeDef(resistencia[i], i);
    e[i] = porcetajeDeseado[i]-porcentaje[i];   
    u[i] = q0[i]*e[i] + q1[i]*e_1[i] + u_1[i];
    e_1[i] = e[i];
    u_1[i] = u[i];    
    uMod[i] = PWM(u[i]);
    
    analogWrite(valvulaPin[i], uMod[i]);
    Serial.print(millis());
    Serial.print(" ");
    Serial.print(porcentaje[i]);
    Serial.print(" ");
    Serial.print(uMod[i]);
    Serial.print(" ");
    Serial.print(porcetajeDeseado[i]);
    Serial.print(" ");
    Serial.print(e[i]);
  }
    Serial.println();

}

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

void serialEvent(){
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while(Serial.available() && !newData) {
    rc = (char)Serial.read();
    if (recvInProgress == true) {
      if (rc != endMarker) {
          receivedChars[ndx] = rc;
          ndx++;
          if (ndx >= numChars) {
              ndx = numChars - 1;
          }
      }
      else {
          receivedChars[ndx] = '\0'; // terminate the string
          recvInProgress = false;
          ndx = 0;
          newData = true;
      }
    }
    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
  strcpy(tempChars, receivedChars);
  parseData();
  newData = false;
}

void parseData() {      // split the data into its parts
  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars,",");      // get the first part - the string
  accion = atoi(strtokIndx); // copy it to messageFromPC

  if(accion == 1){
    //Serial.println("got Stop");
    Stop();
  }
  else if(accion == 2){
    //Serial.println("got Percentage");

    for (int i = 0; i < 5; i++){
      strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
      porcetajeDeseado[i] = atoi(strtokIndx);     // convert this part to an integer
      u[i] = 0;
      u_m[i] = 0;
      analogWrite(motorPin[i],255);

    }
  }
}
