//    Explore9 is an Arduino program intended for execution on the Rabot(TM) toy robot platform.//    Copyright (C) 2020  Daniel D. Stancil and Brian A. Stancil//    This program is free software: you can redistribute it and/or modify//    it under the terms of the GNU General Public License as published by//    the Free Software Foundation, either version 3 of the License, or//    (at your option) any later version.//    This program is distributed in the hope that it will be useful,//    but WITHOUT ANY WARRANTY; without even the implied warranty of//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the//    GNU General Public License for more details.//    You should have received a copy of the GNU General Public License//    along with this program.  If not, see <http://www.gnu.org/licenses/>.// Explore9 enables your Rabot(TM) to autonomously explore his/her environment, avoiding obstacles. // The following two "header" files provide control functions for the azimuth and elevation servos, and pitches for playing music.// The "VarSpeedServo.h" file should be included in any program that controls the Rabot head movement, but the "pitches.h" file// can be omitted if your program does not play tunes.#include "VarSpeedServo.h"#include "pitches.h"// The #define statements below setup definitions for connections between the Arduino and the Rabot shield, and should be included in all// programs.#define TRIGGER_PIN  2  // Arduino pin tied to trigger pin on the ultrasonic sensor.#define ECHO_PIN     4  // Arduino pin tied to echo pin on the ultrasonic sensor.#define MAX_DISTANCE 200 // max distance in cm for ultrasonic sensor#define LEDRed    7  // Arduino pin driving the red LED#define LEDGreen  6 // Arduino pin driving the green LED#define LEDBlue 5 // Arduino pin driving the blue LED#define SPKR    8  // Arduino pin driving the speaker#define EN12  11 // enable for first half-bridge in the motor driver#define EN34  3 // enable for second half-bridge in the motor driver#define M4  13 // half-bridge input 4A (left motor)#define M3  12 // half-bridge input 3A (left motor)#define M2  A2 // half-bridge input 2A (right motor)#define M1  A1 // half-bridge input 1A (right motor)// Adjust these two offsets as needed to make the robot look forward and go straightint azbias = -12; // offset in degrees for azimuth servo. Negative values offset in the CW direction (viewed from above)int offset = -25; // motor speed offset. More negative makes it turn to the right// notes in the melody:int melody[] = {  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};// note durations: 4 = quarter note, 8 = eighth note, etc.:int noteDurations[] = {  4, 8, 8, 4, 4, 4, 4, 4};// These variables are used to set the intensity of the G & B LEDs. // (The Red LED can only be ON or OFF, so we don't have fadeValueR.)int fadeValueG = 0;int fadeValueB = 0;// Each time the Rabot hops, the tail color changes. dtheta determines how much the color changes each time.float pi = 3.1415926;float theta = 0;float dtheta = pi/6; // With this value, the colors will repeat after 6 hops.int LookAngle = 45; // Angle that the head turns to look for obstaclesbool Wait = true;VarSpeedServo myservoAZ;  // define azimuth servoVarSpeedServo myservoEL;  // define elevation servoint speed = 120; // Sets the speed for the motos, 0-255int posAZ = 0;    // variable to store the servo positionint posEL = 0;int ScanSpeed = 60; // Sets speed for turning Rabot's headint i,n;unsigned long valAhead, valRight, valLeft, mysum, val;long randNumber;int azscan = 0;unsigned long pulseWidth = 0;int distance = 50; // Rabot will stop and turn if an obstacle is closer than distance (cm)int turntime = 400; // Duration of Rabot turn to avoid obstacle (mS)int hoptime = 1500; // Duration of hop (mS)void setup() {   // The Arduino pins used for the LEDs and motors to OUTPUT mode    pinMode(LEDRed,OUTPUT);  pinMode(LEDGreen,OUTPUT);  pinMode(LEDBlue, OUTPUT);  pinMode(SPKR,OUTPUT);  pinMode(EN12,OUTPUT);  pinMode(EN34,OUTPUT);  pinMode(M1,OUTPUT);  pinMode(M2,OUTPUT);  pinMode(M3,OUTPUT);  pinMode(M4,OUTPUT);    // Set the modes of the Arduino pins used to control the ultrasonic sensor    pinMode(TRIGGER_PIN,OUTPUT);  pinMode(ECHO_PIN,INPUT);    playSong();    delay(1000); //time to get set up!    myservoEL.attach(10);  // attaches the servo on pin 10 to the servo object  myservoAZ.attach(9);  // attaches the servo on pin 9 to the servo object    // Move the head to look straight ahead    myservoEL.write(90);  myservoAZ.write(90);  delay(5000); // This delay is useful to check the alignment of the servos, and can be reduced or eliminated if this is not needed  // This sets up an output serial line for debugging  pinMode(0,OUTPUT);  Serial.begin(9600);  randomSeed(analogRead(1)); // Generate a new random number seed each time the program is run.  } void playSong(){ // Plays "shave and a haircut, two bits" at startup  // Initiate the LEDs to "off"    digitalWrite(LEDGreen,HIGH); // HIGH turns off the LED  digitalWrite(LEDBlue,HIGH);  digitalWrite(LEDRed,HIGH);    // iterate over the notes of the melody:  for (int thisNote = 0; thisNote < 8; thisNote++) {    // to calculate the note duration, take one second    // divided by the note type.    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.        int noteDuration = 1000 / noteDurations[thisNote];    if (melody[thisNote]>0)    	digitalWrite(LEDRed,LOW); // Turn on the Red LED during the note      tone(SPKR, melody[thisNote], noteDuration); // Play the note    // to distinguish the notes, set a minimum time between them.    // the note's duration + 30% seems to work well:          delay(noteDuration);      digitalWrite(LEDRed,HIGH); // Turn off the Red LED when the note is over      int pauseBetweenNotes = noteDuration * 0.30;      delay(pauseBetweenNotes);    // stop the tone playing:      noTone(SPKR);  }}void loop() {  // Calculate the intensity of the G & B LEDs    fadeValueG = (255.)*sin(theta)*sin(theta);  fadeValueB = (255.)*cos(theta)*cos(theta);  // Set the G & B LEDs to the desired intensity    analogWrite(LEDGreen,fadeValueG);  analogWrite(LEDBlue,fadeValueB);    // Increment theta to change the color next time through the loop    theta = theta + dtheta;    // Check for obstacles before hopping  myservoAZ.write(90-LookAngle+azbias,ScanSpeed,Wait);  valRight = avedist();  delay(200);    myservoAZ.write(90+LookAngle+azbias,ScanSpeed,Wait);  valLeft = avedist();  delay(200);    myservoAZ.write(90+azbias,ScanSpeed,Wait);  val = avedist();  delay(200);  // Print out detected distances to verify operation  // These statements can be commented out except for debugging.    Serial.print("valRight = ");  Serial.println(valRight);  Serial.print(" val = ");  Serial.println(val);  Serial.print(" valLeft = ");  Serial.println(valLeft);  // check for object and choose motion  detectAvoid();  delay(200);  // Go forward again for "hoptime" after turning    //hop(speed,hoptime);  forward(speed);   delay(hoptime);  stopRobot();  delay(50);}// Motion routines for forward, reverse, turns, and stopvoid reverse(int localSpeed) {  digitalWrite(M1,HIGH);  digitalWrite(M2,LOW);  digitalWrite(M4,HIGH);  digitalWrite(M3,LOW);  analogWrite(EN12,localSpeed+offset);  analogWrite(EN34,localSpeed);}void backRight(int localSpeed) {  digitalWrite(M1,HIGH);  digitalWrite(M2,LOW);  digitalWrite(M4,HIGH);  digitalWrite(M3,LOW);  analogWrite(EN12,localSpeed+offset);  analogWrite(EN34,localSpeed/2);}void backLeft(int localSpeed) {  digitalWrite(M1,HIGH);  digitalWrite(M2,LOW);  digitalWrite(M4,HIGH);  digitalWrite(M3,LOW);  analogWrite(EN12,localSpeed/2+offset);  analogWrite(EN34,localSpeed);}void forward(int localSpeed) {  digitalWrite(M1,LOW);  digitalWrite(M2,HIGH);  digitalWrite(M4,LOW);  digitalWrite(M3,HIGH);  analogWrite(EN12,localSpeed+offset/2);  analogWrite(EN34,localSpeed-offset/2);}void forwardLeft(int localSpeed) {  digitalWrite(M1,LOW);  digitalWrite(M2,HIGH);  digitalWrite(M4,LOW);  digitalWrite(M3,HIGH);  analogWrite(EN12,localSpeed+offset/2);  analogWrite(EN34,0.9*localSpeed-offset/2);}void forwardRight(int localSpeed) {  digitalWrite(M1,LOW);  digitalWrite(M2,HIGH);  digitalWrite(M4,LOW);  digitalWrite(M3,HIGH);  analogWrite(EN12,0.9*localSpeed+offset/2);  analogWrite(EN34,localSpeed- offset/2);}void turnLeft(int localSpeed) {  digitalWrite(M1,LOW);  digitalWrite(M2,HIGH);  digitalWrite(M4,HIGH);  digitalWrite(M3,LOW);  analogWrite(EN12,localSpeed+offset/2);  analogWrite(EN34,localSpeed-offset/2);}void turnRight(int localSpeed) {  digitalWrite(M1,HIGH);  digitalWrite(M2,LOW);  digitalWrite(M4,LOW);  digitalWrite(M3,HIGH);  analogWrite(EN12,localSpeed+offset/2);  analogWrite(EN34,localSpeed-offset/2);}void stopRobot() {  analogWrite(M1,0);  analogWrite(M2,0);  analogWrite(M4,0);  analogWrite(M3,0);}unsigned long avedist() {    // Take 5 measurements, then average the ones that are in a plausible range  // (Ultrasonic sensor returns 0 if out of range, or something goes wrong.)  // Return the averaged value    mysum = 0;  i = 0;  for(n=0;n<=4;n++){    val=mydistance();    //Serial.println(val); // This statement can be used for debugging if needed    if ((val>0)&&(val<MAX_DISTANCE)){ //check for plausible range      mysum = mysum + val;      i = i + 1;    }    delay[50];  }  val = mysum/i; // calculate the average  return val;}   void detectAvoid(){    // Check to see if an object is closer than "distance" cm in any of the measured directions    if((val>0 && val<distance)||(valLeft>0 && valLeft<distance)||(valRight>0 && valRight<distance)){    stopRobot();        // Play "uh-oh" and blink RED LED        analogWrite(LEDGreen,255); // Turn off G    analogWrite(LEDBlue,255); // Turn off B    tone(SPKR,NOTE_F6,125); // Play first note "uh"    digitalWrite(LEDRed,LOW); // Turn on R    delay(125);    digitalWrite(LEDRed,HIGH); // Turn off R    delay(150);    tone(SPKR,NOTE_CS6,125); // Play second note "oh"    digitalWrite(LEDRed,LOW); // Turn on R    delay(125);    digitalWrite(LEDRed,HIGH); // Turn off R    delay(50);      // figure out which way to turn          if((valLeft<val)&&(valLeft<valRight)){ // object is on left      Serial.println("object on left"); // For debugging only      turnRight(speed);      delay(turntime);      }    else if((valRight<val)&&(valRight<valLeft)){ // object is on right      Serial.println("object on right");      turnLeft(speed);      delay(turntime);      }    else if((val<valRight)&&(val<valLeft)){ // object is straight ahead      Serial.println("object straight ahead");      	    randNumber = random(2); // Choose a direction to turn at random	    	    if(randNumber == 0)	      {	        turnRight(speed);	        delay(turntime);	      }	    else if(randNumber == 1)	      {	        turnLeft(speed);	        delay(turntime);	      }          }    // Reset G & B LEDs    analogWrite(LEDGreen,fadeValueG);    analogWrite(LEDBlue,fadeValueB);  }  stopRobot();}     unsigned long mydistance() {    // Measure the distance using the ultrasonic transducer    digitalWrite(TRIGGER_PIN,HIGH);  delayMicroseconds(10);  digitalWrite(TRIGGER_PIN,LOW);  pulseWidth = pulseIn(ECHO_PIN, HIGH, 50000);  //Serial.println(pulseWidth);  if (pulseWidth==0){    pulseWidth=11000;  }  return pulseWidth/58.2;}  