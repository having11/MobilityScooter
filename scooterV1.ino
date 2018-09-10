#define JS_X A0 //analog joystick pins
#define JS_Y A1
//Motor pin defines
#define MRB 6 //Motor right
#define MRA 9
#define MLA 10 //Motor left
#define MLB 11

#define NEOPIXEL_PIN 2

#define DEADZONE 20 //A range of +/- this value equals a stop in order to prevent super slow movements. Change as needed

//Each index maps to the other one. So, for instance, the first pin in trigPins and the first pin in echoPins both go to the same HC-SR04
//In order: LEFT | FRONT | RIGHT
const int trigPins[] = {3,5,8}; //HC-SR04 trigger pins  
const int echoPins[] = {4,7,12}; //HC-SR04 echo pins

void setup() {
  initPins();

}

void loop() {
  //checkObstacles(); Not added yet
  updateMotors();

}

void initPins(){
  for(int i=0;i<3;i++){
    pinMode(trigPins[i],OUTPUT);
    pinMode(echoPins[i],INPUT);
  }
  pinMode(MRA,OUTPUT);
  pinMode(MRB,OUTPUT);
  pinMode(MLA,OUTPUT);
  pinMode(MLB,OUTPUT);
}

void updateMotors(){
  int x = map(analogRead(JS_X),0,1023,-255,255);
  int y = map(analogRead(JS_Y),0,1023,-255,255);
  x = constrain(x,-255,255);
  y = constrain(y,-255,255);
  int motor1 = x+y;
  int motor2 = y-x;
  motor1 = constrain(motor1,-255,255);
  motor2 = constrain(motor2,-255,255);
  if(abs(motor1)<=DEADZONE){
    motor1 = 0;
  }
  if(abs(motor2)<=DEADZONE){
    motor2 = 0;
  }
  setMotors(motor1,motor2);
}

void setMotors(int rightVal, int leftVal){
  
  if(rightVal<0){ //Backwards
    analogWrite(MRA,0);
    analogWrite(MRB,rightVal);
  }
  else{
    analogWrite(MRA,rightVal);
    analogWrite(MRB,0);
  }
  if(leftVal<0){ //Backwards
    analogWrite(MLB,leftVal);
    analogWrite(MLA,0);
  }
  else{
    analogWrite(MLA,leftVal);
    analogWrite(MLB,0);
  }
}

long ping(int8_t sensorNumber){
  long duration,distance;
  digitalWrite(trigPins[sensorNumber],LOW);
  delayMicroseconds(2);
  digitalWrite(trigPins[sensorNumber],HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPins[sensorNumber],LOW);
  duration = pulseIn(echoPins[sensorNumber],HIGH);
  distance = (duration/2) / 29.1; //Distance in cm
  return distance;
}

void killMotors(){
  setMotors(0,0);
}
