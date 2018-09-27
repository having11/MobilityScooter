#include <Adafruit_NeoPixel.h>

#define JS_X A0 //analog joystick pins
#define JS_Y A1
//Motor pin defines
#define AN1 6 //Analog input for motor left
#define AN2 9 //Analog input for motor right
#define IN1 10 //Direction for motor left
#define IN2 11 //Direction for motor right

#define NEOPIXEL_PIN 2 //Neopixel data pin
#define LED_NUM 60 //How many LEDs in the strip? It's 60 LEDs per meter, so just change this based on how many meters you bought

#define SAFE_RANGE 30 //Anything above or equal to 30cm
#define WARNING_RANGE 9 //Anything within 10-30 cm
#define STOP_RANGE 8 //Anything less than or equal to 8cm

#define POLL_RATE 500 //How often to check distances in ms
#define COOLDOWN_TIME 2000 //Wait this many milliseconds until the motors re-enable if they go into "STOP" mode.
#define DEADZONE 20 //A range of +/- this value equals a stop in order to prevent super slow movements. Change as needed

Adafruit_NeoPixel led_strip = Adafruit_NeoPixel(LED_NUM,NEOPIXEL_PIN,NEO_GRB+NEO_KHZ800);

const uint32_t colors[] = {led_strip.Color(0,200,0),led_strip.Color(200,200,0),led_strip.Color(240,0,0)};

bool motor_enable = true;

//Each index maps to the other one. So, for instance, the first pin in trigPins and the first pin in echoPins both go to the same HC-SR04
//In order: LEFT | FRONT | RIGHT
const uint8_t trigPins[] = {3,5,8}; //HC-SR04 trigger pins  
const uint8_t echoPins[] = {4,7,12}; //HC-SR04 echo pins

long last_time = 0,last_disable=0;

enum Warnings{
  SAFE=0, //Green
  WARNING=1, //Yellow
  STOP=2 //Red
};

void setup() {
  initPins();
  led_strip.begin(); //start the neopixels
  led_strip.show();
}

void loop() {
  if(millis()-last_time>POLL_RATE){ //See if it's time to check distances and compare them / update the lights
    checkObstacles();
    last_time = millis();
  }
  if(motor_enable){ //If you can move, move
    updateMotors();
  }
  else{
    if(millis()-last_disable>COOLDOWN_TIME){ //If enough time has passed allow the motors to move again
      motor_enable = true; //re-enable
    }
    else{
      killMotors(); //Or else just keep them off
    }
  }

}

void checkObstacles(){
  long distance;
  for(int sensor=0;sensor<3;sensor++){ //Go through each HC-SR04 in this order: Left -> Front -> Right
    distance = ping(sensor); //Get the distance of that particular sensor
    if(distance>=SAFE_RANGE){ //If in the safe range make the lights green
      setLights(SAFE,sensor);
    }
    else if(distance<30&&distance>=WARNING_RANGE){ //If in the warning range make the lights yellow
      setLights(WARNING,sensor);
    }
    else if(distance<WARNING_RANGE&&distance>=0){ //If in the stop range make the lights red
      setLights(STOP,sensor);
      motor_enable = false; //Disable the motors
      last_disable = millis(); //Start the cooldown period
    }
  }
  led_strip.show(); //Show the updated colors
}

void setLights(Warnings warning, int sensor){
  uint32_t color = colors[warning]; //Get the corresponding color from the array
  int thirdOfLights = LED_NUM / 3; //So for instance: 60/3= 20 LEDs per side/sensor
  for(int i=(thirdOfLights*sensor);i<thirdOfLights*(sensor+1);i++){ //Set that third to specified color
    led_strip.setPixelColor(i,color);
  }
}

void initPins(){
  for(int i=0;i<3;i++){ //Set all HC-SR04 pins accordingly
    pinMode(trigPins[i],OUTPUT);
    pinMode(echoPins[i],INPUT);
  }
  //Set motor pins to output
  pinMode(AN1,OUTPUT);
  pinMode(AN2,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  analogWrite(AN1,0);
  analogWrite(AN2,0);
  digitalWrite(IN1,0);
  digitalWrite(IN2,0);
  delay(2000); //Wait for motor driver to boot up
}

void updateMotors(){
  int x = map(analogRead(JS_X),0,1023,-255,255); //Get joystick axis values
  int y = map(analogRead(JS_Y),0,1023,-255,255);
  x = constrain(x,-255,255); //Constrain them to prevent outliers
  y = constrain(y,-255,255);
  int motor1 = y+x; //Do some math to get the right values for the wheels
  int motor2 = y-x;
  motor1 = constrain(motor1,-255,255); //Constrain them again to prevent outliers
  motor2 = constrain(motor2,-255,255);
  if(abs(motor1)<=DEADZONE){ //Check if motor values are in the deadzone range
    motor1 = 0;
  }
  if(abs(motor2)<=DEADZONE){
    motor2 = 0;
  }
  setMotors(motor1,motor2); //Set the motors to the right values
}

void setMotors(int rightVal, int leftVal){
  //Works like a tank drive system
  if(rightVal<0){ //Backwards
    digitalWrite(IN2,1); //Motor counter-clockwise
    analogWrite(AN2,abs(rightVal));
  }
  else{
    digitalWrite(IN2,0); //Motor clockwise
    analogWrite(AN2,rightVal);
  }
  if(leftVal<0){ //Backwards
    analogWrite(AN1,abs(leftVal));
    digitalWrite(IN1,1);
  }
  else{
    analogWrite(AN1,leftVal);
    digitalWrite(IN1,0);
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
  duration = pulseIn(echoPins[sensorNumber],HIGH); //Wait for 'ping' to return and measure the time it took
  distance = (duration/2) / 29.1; //Distance in cm
  return distance;
}

void killMotors(){ //Turn the motors off
  setMotors(0,0);
}
