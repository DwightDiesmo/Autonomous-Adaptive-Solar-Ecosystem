/*
Team: Optimized Solar Energy
Members: Dwight Diesmo
         Justin Killam
         Jason Babayev
         Riley Turner
Description: System firmware for the optimized
*/

//Servo library for driving servo motor
#include <Servo.h>

//Define statments for the pins and increment value
#define increment 50
#define ServoPin 3
#define PumpPin 4

//Variabes for photoresistor calculations
int leftPR;
int rightPR;
int deltaPR;

//Position variables
int plusInc,minusInc;

//Initial position of solar panel
uint16_t curPos=1500;

//Variable 
bool trigger=false;
Servo MyServo;
void setup() {
  
  ////Code used for testing. Commented out.
  //Serial.begin(9600);

  //Servo Initialization
  MyServo.attach(ServoPin);
  pinMode(PumpPin,OUTPUT);
  digitalWrite(PumpPin,LOW);
}

void loop() {

    leftPR = analogRead(A0);
    rightPR = analogRead(A1);

////Code used for testing. Commented out.   
//    Serial.println(curPos);
//    Serial.println(trigger);
//    Serial.println(" ");

    //calculating the delta between the photoresistor measurements
    deltaPR = rightPR - leftPR;

    //Precalculating the incremented and decremented values of the panel position
    plusInc=curPos+increment;
    minusInc=curPos-increment;
      //Primary sensor decision algorithim
      //If the right photor sensor is recieving more light than the left solar panel rotate right.
      if (deltaPR <= -100) {
          //Setting servo position maximum value
          if(plusInc>1800){
            curPos=1800;
            trigger=LOW;
          }
          //Trigger the movement and increment the position
          else {
            curPos=plusInc;
            trigger=HIGH;
          }
      }
      //If the right photor sensor is recieving more light than the left solar panel rotate right.
      else if (deltaPR >= 100) {
          //Setting servo position minimum value
          if(minusInc<1200){
            curPos=1200;
            trigger=LOW;
          }
          //Trigger the movement and decrement the position
          else {
            curPos=minusInc;
            trigger=HIGH;
          }
      }
      //Turn off the system
      else{
          trigger=LOW;
      }
      if(trigger){
          MyServo.writeMicroseconds(curPos);
          digitalWrite(PumpPin,HIGH);
      }
      else{
          digitalWrite(PumpPin,LOW);
      }
      delay(200);
}
