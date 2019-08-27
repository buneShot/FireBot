#include<MotorDriver.h>
// motor_object.motor(pin, DIRECTION, SPEED); This is used to run the motor
#include<Servo.h>
/////////////////////////////////////////////////////////////////////////
// Ultrasonic Sensor
#define ECHO 53 //INPUT  - to be received by the board
#define TRIG 52 //OUTPUT WAVE HIGH

int distance = 0; //To hold Distance in loop() & getDistance() method
bool normalLongSense;
int left; //for searchFlamesAllAround()
int right;//for searchFLamesAllAround()

int normDistance;
int duration;

int getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  duration = pulseIn(ECHO, HIGH);

  distance = duration/58;
  if(duration == 0){
    Serial.println("No Pulse");
  }
  else{
    return distance;
  }
}

////////////////////////////////////////////////////////////////////////
//Flame Sensors - Long and Short range
#define FLONG 50
#define FSHORT 51
bool longSense;
bool shortSense;
bool normLongSense;
bool isFlamePutOff = false;
bool checkFlame(int sense) {
  if (!digitalRead(sense)) {
    return true;
  }
  else {
    return false;
  }
}
////////////////////////////////////////////////////////////////////////

//Servo that has Flame and Ultrasonic Sensors.
#define HEAD 10
Servo headCon; //Head Controlling Object
#define HEADNORMAL 90 //position of head 
#define MINANGLE 40
#define MAXANGLE 140
int pos = 90;
int direct;

/////////////////////////////////////////////////////////////////////
//Buzzer
#define BUZZ 48//Buzzer's Digital Pin on 48 that has to be set to OUTPUT
#define AINT 100// Alert When Flame detected || when cart is Moving
//Buzzer Activation Method that takes the time interval in Milliseconds after which the sound goes low.
void beep() {
  digitalWrite(BUZZ, HIGH);
}
void stopBeep() {
  digitalWrite(BUZZ, LOW);
}
void alertSound(int interval) {
  beep();
  delay(interval);
  stopBeep();
  delay(interval);
}

////////////////////////////////////////////////////////////////////////
//Wheels
MotorDriver leftWheel;
MotorDriver rightWheel;
MotorDriver pump;
#define LEFT 1 //m1 pin
#define RIGHT 4 // m4 pin  
#define PUMP 2 //DC Motor Pump

//Bot Movement
void moveAhead() {
  leftWheel.motor(LEFT, FORWARD, 225);
  rightWheel.motor(RIGHT, FORWARD, 225);
}
void halt() {
  leftWheel.motor(LEFT, FORWARD, 0);
  rightWheel.motor(RIGHT, FORWARD, 0);
}
void turnLeft() {
  leftWheel.motor(LEFT, BACKWARD, 200);
  rightWheel.motor(RIGHT, FORWARD, 200);
}
void turnRight() {
  leftWheel.motor(LEFT, FORWARD, 200);
  rightWheel.motor(RIGHT, BACKWARD, 200);
}
void reverse() {
  leftWheel.motor(LEFT, BACKWARD, 200);
  rightWheel.motor(RIGHT, BACKWARD, 200);
}

void startPump(){
  pump.motor(PUMP, FORWARD, 225);
}

void stopPump(){
  pump.motor(PUMP, FORWARD, 0);
}

void startAllMotors(){
  leftWheel.motor(LEFT, FORWARD, 0);
  rightWheel.motor(RIGHT, FORWARD, 0);
  pump.motor(PUMP, FORWARD, 0);
}

void stopEverything(){
  leftWheel.motor(LEFT, RELEASE, 0);
  rightWheel.motor(RIGHT, RELEASE, 0);
  pump.motor(PUMP, RELEASE, 0);
}
///////////////////////////////////////////////////////////////////////////
bool isNew = true;
int searchAllAround() {

  bool leftLongSense;
  bool rightLongSense;
  bool normalLongSense;
  beep();
  delay(200);
  stopBeep();
  delay(200);
  
  for (; pos >= MINANGLE; pos--) {
    headCon.write(pos);
    delay(20);
  }
  pos++;
  delay(30);
  right = getDistance();
  rightLongSense = checkFlame(FLONG);
  Serial.print("Right: ");
  Serial.println(right);
  Serial.print("Right Flame: ");
  Serial.println(rightLongSense);

  for (; pos <= MAXANGLE; pos++) {
    headCon.write(pos);
    delay(20);
  }
  pos--;
  delay(30);
  left = getDistance();
  leftLongSense = checkFlame(FLONG);
  Serial.print("Left: ");
  Serial.println(left);
  Serial.print("Left Flame: ");
  Serial.println(leftLongSense);

  toNormalHead();
  delay(30);
  normalLongSense = checkFlame(FLONG);
  normDistance = getDistance();
  if(leftLongSense || rightLongSense || normLongSense){
    if(leftLongSense){
      return 1;
    }
    else if(rightLongSense){
      return 2;
    }
    else{
      return 3;
    }
  }
  else{
    if(normDistance > 23){
        return 3;
    }
    else{
        return (left>right)?1:2;
    }
  }
  
}

void toNormalHead() {
  if (pos >= 90) {
    for (; pos >= HEADNORMAL; pos--) {
      headCon.write(pos);
      delay(20);
    }
    pos++;
  }
  else {
    for (; pos <= HEADNORMAL; pos++) {
      headCon.write(pos);
      delay(20);
    }
    pos--;
  }
}

void moveCart(int dir){

  if(dir == 0){
    turnLeft();
    Serial.println("Reverse");
    delay(1500);
  }
  else if(dir == 1){
      reverse();
      delay(200);
      turnLeft();
      delay(250);
      Serial.println("Turning Left");
      halt();
  }
  else if(dir == 2){
      reverse();
      delay(200);
      turnRight();
      delay(250);
      Serial.println("Turning Right");
      halt();
  }
  else{
      moveAhead();
      Serial.println("Moving Ahead -- After Searching for Flames...");
  }
}
void firstCheck(){
  if(isNew){
    isNew = false;
    direct = searchAllAround();
    moveCart(direct);
  }
}
/////////////////////////////////////////////////////////////
//Initialization Code
void setup() {
  //Buzzer pin to OUTPUT
  pinMode(BUZZ, OUTPUT);

  //Servos buddy
  headCon.attach(HEAD);
  headCon.write(HEADNORMAL);

  //IR Sensors
  pinMode(FLONG, INPUT);
  pinMode(FSHORT, INPUT);

  //UltraSonic Sensors
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  //Start All Motors
  startAllMotors();
  
  //Off All Sounds
  stopBeep();
  delay(20);
  distance = getDistance();
  delay(100);

  
  Serial.begin(9600);
}

//Main Loop
void loop(){
  stopBeep();  
  stopPump();
  shortSense = checkFlame(FSHORT);
  Serial.print("Distance: ");
  Serial.println(distance);
  
    if(distance<23 || shortSense){
      halt();
      if(shortSense){
        beep();
        startPump();
        delay(5000);
        Serial.println("Flame!!!! Starting Pump");
        shortSense = checkFlame(FSHORT);
        if(!shortSense){ 
          halt();
          stopPump();
          Serial.println("Stopping Pump!!!");
          stopBeep();
        }
      }
     else{
        Serial.println("Obstacle Here!! So, I'm Searching");
        beep();
        direct = searchAllAround();
        moveCart(direct);
     }  
    }
    else{
      moveAhead();
    }
    delay(30);
    distance = getDistance();
}
