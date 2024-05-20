#include <MeMCore.h>

#define TURNING_TIME_MS 340
#define ULTRASONIC 12
#define TIMEOUT 2000
#define SPEED_OF_SOUND 340

#define INPUT_A A0
#define INPUT_B A1
#define IR_RECEIVER A3
#define LDR_RECEIVER A2

#define RGBWait 200
#define LDRWait 10

int status = 0;

MeBuzzer buzzer;
MeLineFollower lineFinder(PORT_2);
MeDCMotor leftMotor(M1);
MeDCMotor rightMotor(M2);
uint8_t motorSpeed = 250;

float colourArray[] = {0,0,0};
/*
colourArray[0] = R value for unknown colour detected
colourArray[1] = G value for unknown colour detected
colourArray[2] = B value for unknown colour detected
*/

float whiteArray[] = {979,991,978}; //White calibration values
float blackArray[] = {947,936,892}; //Black calibration values
float greyDiff[] = {32,55,86}; //whiteArray[] - blackArray[]
char colourStr[3][5] = {"R = ", "G = ", "B = "};

float colours[6][3] = {{220, 122, 139}, {223, 176, 148}, {79, 180, 127}, {127, 215, 237}, {151, 171, 198}, {231, 255, 255}};
/*
colours[0] = RGB values for red
colours[1] = RGB values for orange
colours[2] = RGB values for green
colours[3] = RGB values for blue
colours[4] = RGB values for purple
colours[5] = RGB values for white
*/

void celebrate() {  // Code for playing celebratory tune
  buzzer.tone(392, 200);
  buzzer.tone(523, 200);
  buzzer.tone(659, 200);
  buzzer.tone(784, 200);
  buzzer.tone(659, 150);
  buzzer.tone(784, 400);
  buzzer.noTone();
}
void stopMotor() {  // Code for stopping motor
  leftMotor.stop(); // Stop left motor 
  rightMotor.stop(); // Stop right motor 
}
void moveForward() {  // Code for moving forward for some short interval
  leftMotor.run(-motorSpeed); // Negative: wheel turns anti-clockwise 
  rightMotor.run(motorSpeed); // Positive: wheel turns clockwise 
}
void turnRight(int time) {
  leftMotor.run(-motorSpeed);
  rightMotor.run(-motorSpeed);
  delay(time);
}
void turnLeft(int time) {
  leftMotor.run(motorSpeed);
  rightMotor.run(motorSpeed);
  delay(time);
}
void uTurn() {
  leftMotor.run(-motorSpeed);
  rightMotor.run(-motorSpeed);
  delay(2*TURNING_TIME_MS - 80);
}
void doubleLeftTurn() {   // Code for double left turn
  turnLeft(TURNING_TIME_MS-15);
  stopMotor();
  moveForward();
  delay(650);
  stopMotor();
  delay(100);
  turnLeft(TURNING_TIME_MS+15);
  stopMotor();
  delay(10);
}
void doubleRightTurn() {  // Code for double right turn
  turnRight(TURNING_TIME_MS-10);
  stopMotor();
  moveForward();
  delay(690);
  stopMotor();
  delay(100);
  turnRight(TURNING_TIME_MS);
  stopMotor();
  delay(10);
}
void nudgeLeft() {  // Code for nudging slightly to the left for some short interval
  leftMotor.run(0);
  rightMotor.run(230);
}
void nudgeRight() {   // Code for nudging slightly to the right for some short interval
  leftMotor.run(-230);
  rightMotor.run(0);
}
void shineIR() {   // Code for turning on the IR emitter only
  analogWrite(INPUT_A, 255);
  analogWrite(INPUT_B, 255);
}
void shineRed() {   // Code for turning on the red LED only
  analogWrite(INPUT_A, 0);
  analogWrite(INPUT_B, 0);
}
void shineGreen() {   // Code for turning on the green LED only
  analogWrite(INPUT_A, 255);
  analogWrite(INPUT_B, 0);
}
void shineBlue() {  // Code for turning on the blue LED only
  analogWrite(INPUT_A, 0);
  analogWrite(INPUT_B, 255);
}

void decoder_state(int state) {   //controls the 2-to-4 decoder
  if (state == 0) {  //turn on RED LED
    shineRed();
  }
  else if (state == 1) {  //turn on GREEN LED
    shineGreen();
  }
  else if (state == 2) {  //turn on BLUE LED
    shineBlue();
  }
  else if (state == 3) {  //turn on IR emitter (turn off all LEDs)
    shineIR();
  }
}

float square(float a) {
  return (a*a);
}

void setBalance()   //for calibration of white and black
{
  //set white balance
  Serial.println("Put White Sample For Calibration ...");
  delay(5000);           //delay for five seconds for getting sample ready
  //scan the white sample.
  //go through one colour at a time, set the maximum reading for each colour -- red, green and blue to the white array
  for(int i = 0; i <= 2; i++) {
    decoder_state(i);
    delay(RGBWait);
    whiteArray[i] = getAvgReading(5);         //scan 5 times and return the average, 
    Serial.println(whiteArray[i]);
    decoder_state(3);
    delay(RGBWait);
  }
  //done scanning white, time for the black sample.
  //set black balance
  Serial.println("Put Black Sample For Calibration ...");
  delay(5000);     //delay for five seconds for getting sample ready 
  //go through one colour at a time, set the minimum reading for red, green and blue to the black array
  for(int i = 0; i <= 2; i++){
    decoder_state(i);
    delay(RGBWait);
    blackArray[i] = getAvgReading(5);
    Serial.println(blackArray[i]);
    decoder_state(3);
    delay(RGBWait);
  //the differnce between the maximum and the minimum gives the range
    greyDiff[i] = whiteArray[i] - blackArray[i];
  }

  //delay another 5 seconds for getting ready colour objects
  Serial.println("Colour Sensor Is Ready.");
  delay(5000);
}

int getAvgReading(int times)  //get average readings
{      
  //find the average reading for the requested number of times of scanning LDR
  int reading;
  int total = 0;
  //take the reading as many times as requested and add them up
  for(int i = 0; i < times; i++){
    reading = analogRead(LDR_RECEIVER);
    total = reading + total;
    delay(LDRWait);
  }
  //calculate the average and return it
  return total/times;
}

void setup()
{
  // Configure pinMode for A0, A1, A2, A3
  pinMode(INPUT_A, OUTPUT);
  pinMode(INPUT_B, OUTPUT);
  pinMode(IR_RECEIVER, INPUT);
  pinMode(LDR_RECEIVER, INPUT);
  Serial.begin(9600);
  decoder_state(3);
  //setBalance();
}

void loop()
{
  if (analogRead(A7) < 100) { // If push button is pushed, the value will be very low
  status = 1 - status; // Toggle status
  delay(500); // Delay 500ms so that a button push won't be counted multiple times. 
  }

  if (status == 1) { // run mBot only if status is 1
    int sensorState = lineFinder.readSensors(); // read the line sensor's state 
    if (sensorState == S1_IN_S2_IN) {   //black line detected
      stopMotor();
      for (int c = 0; c <= 2; c++) {    
        //Serial.print(colourStr[c]);
        decoder_state(c); //turn ON the LED, red, green or blue, one colour at a time.
        delay(RGBWait);
        //get the average of 5 consecutive readings for the current colour and return an average 
        colourArray[c] = getAvgReading(5);
        /*the average reading returned minus the lowest value divided by the maximum possible range, multiplied by 255 will give a value between 0-255, 
        representing the value for the current reflectivity (i.e. the colour LDR is exposed to)*/
        colourArray[c] = ((colourArray[c] - blackArray[c])/(greyDiff[c]))*255;
        decoder_state(3);  //turn off the current LED colour
        delay(RGBWait);
        //Serial.println(int(colourArray[c])); //show the value for the current colour LED, which corresponds to either the R, G or B of the RGB code
      }

      int min = 0;  //Variable to identify the colour with the shortest distance to the unknown colour
      long minDist = 195075; //255^2 + 255^2 + 255^2
      for (int i = 0; i < 6; i++) {
        long distance = square(colours[i][0]-colourArray[0]) + square(colours[i][1]-colourArray[1]) + square(colours[i][2]-colourArray[2]); //Calculate Euclidean distance between unknown colour and each of the 6 colours
        //Serial.println(distance);
        if (distance < minDist) {
          minDist = distance;  //Update smallest distance
          min = i;  //Update colour with the smallest distance
        }
      }
      if (min == 0) {
        //Serial.println("red");
        stopMotor();
        turnLeft(TURNING_TIME_MS-5);
        stopMotor();
        delay(50);
      }
      else if (min == 1) {
        //Serial.println("orange");
        stopMotor();
        uTurn();
        stopMotor();
        delay(200);
      }
      else if (min == 2) {
        //Serial.println("green");
        turnRight(TURNING_TIME_MS-5);
        stopMotor();
        delay(50);
      }
      else if (min == 3) {
        //Serial.println("blue");
        doubleRightTurn();
        delay(50);
      }
      else if (min == 4) {
        //Serial.println("purple");
        doubleLeftTurn();
        delay(50);
      }
      else {
        //Serial.println("white");
        stopMotor();
        celebrate();
      }
    }
    else {  //no black line detected, continue moving
      moveForward();

      pinMode(ULTRASONIC, OUTPUT);
      digitalWrite(ULTRASONIC, LOW); 
      delayMicroseconds(2); 
      digitalWrite(ULTRASONIC, HIGH); 
      delayMicroseconds(10); 
      digitalWrite(ULTRASONIC, LOW);

      pinMode(ULTRASONIC, INPUT);
      long duration =  pulseIn(ULTRASONIC, HIGH, TIMEOUT); // Read ultrasonic sensing distance
      float dist = (duration / 2.0 / 1000000) * SPEED_OF_SOUND * 100; //Converts the distance to centimetres
      //Serial.println(dist);
      if (dist > 0.0 && dist < 8.0) { //Checks whether mBot is too close to left wall
        nudgeRight();
      }
      else if (dist > 13.0) {
        decoder_state(3);   //Turn on IR emitter
        delay(5);
        int val = analogRead(IR_RECEIVER);  //Read reflected IR and ambient IR
        decoder_state(0);   //Turn off IR emitter
        delay(5);
        int IR_diff = analogRead(IR_RECEIVER) - val; //Read ambient IR and calculate the difference between the two readings
        //Serial.println(IR_diff);
        if (IR_diff > 350) {  //Checks whether mBot is too close to right wall
          nudgeLeft();
        }
      }
    }
    delay(20); // decision making interval (in milliseconds) 
  }
}