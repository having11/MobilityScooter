#include <Adafruit_NeoPixel.h>

#define JS_X A0 //analog joystick pins
#define JS_Y A1
//Motor pin defines
#define MRB 6 //Motor right
#define MRA 9
#define MLA 10 //Motor left
#define MLB 11

#define NEOPIXEL_PIN 2 //Neopixel data pin
#define LED_NUM 60 //How many LEDs in the strip? It's 60 LEDs per meter, so just change this based on how many meters you bought

#define SAFE_RANGE 30 //Anything above or equal to 30cm
#define WARNING_RANGE 9 //Anything within 10-30 cm
#define STOP_RANGE 8 //Anything less than or equal to 8cm

#define POLL_RATE 500 //How often to check distances in ms
#define DEADZONE 20 //A range of +/- this value equals a stop in order to prevent super slow movements. Change as needed

Adafruit_NeoPixel led_strip = Adafruit_NeoPixel(LED_NUM,NEOPIXEL_PIN,NEO_GRB+NEO_KHZ800);

const uint32_t colors[] = {led_strip.Color(0,200,0),led_strip.Color(200,200,0),led_strip.Color(240,0,0)};

//Each index maps to the other one. So, for instance, the first pin in trigPins and the first pin in echoPins both go to the same HC-SR04
//In order: LEFT | FRONT | RIGHT
const uint8_t trigPins[] = {3,5,8}; //HC-SR04 trigger pins  
const uint8_t echoPins[] = {4,7,12}; //HC-SR04 echo pins

long last_time = 0;

enum Warnings{
  SAFE=0, //Green
  WARNING=1, //Yellow
  STOP=2 //Red
};

void setup() {
  initPins();
  led_strip.begin();
  led_strip.show();
}

void loop() {
  if(millis()-last_time>POLL_RATE){
    checkObstacles();
    last_time = millis();
  }
  updateMotors();

}

void checkObstacles(){
  long distance;
  for(int sensor=0;sensor<3;sensor++){
    distance = ping(sensor);
    if(distance>=SAFE_RANGE){
      setLights(SAFE,sensor);
    }
    else if(distance<30&&distance>=WARNING_RANGE){
      setLights(WARNING,sensor);
    }
    else if(distance<WARNING_RANGE&&distance>=0){
      setLights(STOP,sensor);
    }
  }
  led_strip.show();
}

void setLights(Warnings warning, int sensor){
  uint32_t color = colors[warning];
  int thirdOfLights = LED_NUM / 3;
  for(int i=(thirdOfLights*sensor);i<thirdOfLights*(sensor+1);i++){ //Set that third to specified color
    led_strip.setPixelColor(i,color);
  }
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
  unsigned long duration;
  long distance;
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
