#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <Servo.h> 
//rf
RH_ASK driver(2000, 2, 4, 16); // ESP8266 or ESP32: do not use pin 11 transmitting on gipo4-D2
//ultrasonic sensor
// defines pins numbers
const int trigPin = 14;  //D5
const int echoPin = 12;  //D6

/////Testing for servo://///////////////////
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
const int checkpin=2;
const int servopin=5;


// defines variables ultrasonic
long duration;
int distance;
//variable for water tank water control
  float total_height=26;//this variable needs to be exposed to the web-page so that it can be changed.
  float distance_waterlevel;//indicates the distance between the ultrasonic sensor and the waterlevel.
  float percentage_water_in_tank;
  int upper_level_percentage_water_in_tank=80;//should also be expose to the internet for change
  int lower_percentage_water_in_tank=20;//expose to the internet
  float current_height;
  //variables for pump control- (:note the pump is controlled via an rf transmitter so the variables for the rf will also be included here)

void setup() {
  // put your setup code here, to run once:
pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
pinMode(echoPin, INPUT); // Sets the echoPin as an Input`vx
pinMode(checkpin,INPUT);
if (!driver.init())
         Serial.println("init failed");
Serial.begin(9600); // Starts the serial communication

}

void loop() {
 servoFunction();
 ultrasonicFunction(); 
 relayFunction();
 
  
 Serial.println("looping");
}

void relayFunction(){
  if(percentage_water_in_tank<lower_percentage_water_in_tank){
    const char *msg = "1";
    driver.send((uint8_t *)msg, strlen(msg));
    Serial.println("low water");
    //driver.waitPacketSent();
  
    
  }
  else if (percentage_water_in_tank>upper_level_percentage_water_in_tank){
    //turn off the water pump
    const char *msg = "0";
    driver.send((uint8_t *)msg,strlen(msg));
    Serial.println("tank full");
    //driver.waitPacketSent(); 
  } 
}
void ultrasonicFunction(){
  // put your main code here, to run repeatedly:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  distance_waterlevel=distance;
  current_height=total_height-distance_waterlevel;//indicates the current height of the water level.
  percentage_water_in_tank=(current_height/total_height)*100;
 Serial.println(""+String(total_height)+"-"+String(distance_waterlevel)+"="+String(current_height));
 Serial.println(String(percentage_water_in_tank)+"%");
 delay(100);
}


void servoFunction(){
  if(digitalRead(checkpin)){
  if(myservo.read()==120){
  return;
  }
  myservo.attach(servopin);
  myservo.write(120);
  delay(300);
  Serial.print("Supply output is on");
 }
 else if(!digitalRead(checkpin)){
  if(myservo.read()==45){
  return;
  }
  myservo.attach(servopin);
  myservo.write(45);
  delay(300);
  
  Serial.print("Supply output is off");
 }
Serial.println("servo position is currently at "+String(myservo.read()));
}

