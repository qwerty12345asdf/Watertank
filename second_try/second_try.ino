#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
//rf
RH_ASK driver(2000, 2, 4, 5); // ESP8266 or ESP32: do not use pin 11 transmitting on gipo4
//ultrasonic sensor
// defines pins numbers
const int trigPin = 14;  //D5
const int echoPin = 12;  //D6

// defines variables ultrasonic
long duration;
int distance;
//variable for water tank water control
  float total_height=47;//this variable needs to be exposed to the web-page so that it can be changed.
  float distance_waterlevel;//indicates the distance between the ultrasonic sensor and the waterlevel.
  float percentage_water_in_tank;
  int upper_level_percentage_water_in_tank=80;//should also be expose to the internet for change
  int lower_percentage_water_in_tank=20;//expose to the internet
  float current_height;
//variables for pump control- (:note the pump is controlled via an rf transmitter so the variables for the rf will also be included here)
//variables for time.
const current_time;
const start_time;
const end_time;
//variables for web-page toggle pins
const motor_toggle;
const valve_toggle;
void setup() {
  // put your setup code here, to run once:
pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
pinMode(echoPin, INPUT); // Sets the echoPin as an Input`
if (!driver.init())
         Serial.println("init failed");
Serial.begin(9600); // Starts the serial communication

}

void loop() {
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
 Serial.println(String(percentage_water_in_tank)+":");
 delay(100);
 //display current waterlevel on page
 //check for time and date
 if(current_time>start_time && percentage_water_in_tank<upper_level_percentage_water_in_tank && current_time<end_timedf){
	 //turn-off the valve output. this is not something we want ideally so we will actuall keep this open 
	 //but the other taps connected to the main supply may be on or off.
	 //write code
	 
	while(percentage_water_in_tank<upper_level_percentage_water_in_tank){
	//turn on the water pump
    const char *msg = "1";
    driver.send((uint8_t *)msg,strlen(msg));
    Serial.println("filling water");
	}
	Serial.println("tank full");
	
  } 
 if(current_time>end_time){
	//we need to still keep sending 0 to the pump to tell it to remain off.
    //turn off the water pump
    const char *msg = "0";
    driver.send((uint8_t *)msg,strlen(msg));
    Serial.println("tank full");
    //driver.waitPacketSent(); 
	//the output valve will be kept on now
 }
 //manual motor control
 if(motor_toggle.equals("true")){
	while(percentage_water_in_tank<upper_level_percentage_water_in_tank){
	//turn on the water pump
    const char *msg = "1";
    driver.send((uint8_t *)msg,strlen(msg));
    Serial.println("filling water");
	}
 }
 else if(motor_toggle.equals("false")){
	 //turn off the water pump
    const char *msg = "0";
    driver.send((uint8_t *)msg,strlen(msg));
 }
 if(valve_toggle.equals("true")){
 //turn on the valve //by default true
 }
 else if(valve_toggle.equals("true")){
 //turn off the valve//ie servo motor code.
 
 }
 //display and updation of values on the web page.
	//web page contains forms and toggle pins and a submit option
	//web page contains forms and toggle pins and a submit option
 
 delay(1000);
 
}
