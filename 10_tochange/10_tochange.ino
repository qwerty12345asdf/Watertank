#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include <ESP8266mDNS.h>        // Include the mDNS library
#include "StringSplitter.h"
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
//rf integration in this sketch got it to turn on and off an led at the out put by polling a pin check pin d7

//rf integration
RH_ASK driver(2000,15,2); // ESP8266 or ESP32: do not use pin 11
int motorstate =0;//initialize motor state to off

//Time variables with ntp settings
String on_time,off_time;
String d_on_time,d_off_time; 
int h_on,m_on;
int h_off,m_off;
String s_time_date ="";
// NTP Servers:
static const char ntpServerName[] = "time.nist.gov";
const int timeZone = 5;     // Indian extra 30 added in calculation function getNtpTime()
const int timeZoneMin=28;
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t getNtpTime();
String digitalClockDisplay();
String valDigits(int digits);
void sendNTPpacket(IPAddress &address);
time_t prevDisplay = 0; // when the digital clock was displayed

//declaring some variables for website creation 
String webSite,javaScript,XML,Argument_Name;
int serverPort=80;
ESP8266WebServer server(serverPort);
//Variables for waterlevel sensing using ultrasonic sensor
long duration;
int distance;
float total_height;//this variable needs to be exposed to the web-page so that it can be changed.
float distance_waterlevel;//indicates the distance between the ultrasonic sensor and the waterlevel.
float percentage_water_in_tank;
int upper_level_percentage_water_in_tank;//should also be expose to the internet for change
int lower_level_percentage_water_in_tank;//expose to the internet
float current_height;
const int echoPin = 16;  //D0
const int trigPin = 5;  //D1

//Variables to control the servo
Servo myservo;
int servo_control_pin=4;//GIPO4 OR D2 pin


//some variable for internet connection
const char* ssid="FD 47";//the wifi network i am connecting to 
const char* pass="123456789012";//the password of the wifi netw.

//adding domain names refer:https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html

///////////////////////SETUP//////////////////////////////////////////////////
void setup() {
  //testing for rf transmission
  if (!driver.init())
         Serial.println("init failed");
    //pinMode(pincheck,INPUT);        
  
  //ultrasonic sensor setup
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input`
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("connecting to");
  Serial.print(ssid);
  WiFi.begin(ssid,pass);
  while(WiFi.status()!= WL_CONNECTED){
  delay(500);
  Serial.print(".");
}
Serial.println("Wifi connected");
  //Setup to receive ntp time from the internet
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  
  //Start the server
  server.begin();
  Serial.println("Server started");
  //print the ip address
  Serial.println("Use this url to connect:");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.print(serverPort);
  Serial.print("/");
  delay(1000);
  //strating mdns domain name 
   if (!MDNS.begin("watertankcontrol")) {             // Start the mDNS responder for watertankcontrol.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
  
  // Next define what the server should do when a client connects
  server.on("/", handleWebsite); // The client connected with no arguments e.g. http:192.160.0.40/
  server.on("/xml",handleXML);
  //server.on("/result", ShowClientResponse);
 }
/////////////////////////LOOP////////////////////////////////////////
void loop() {
  thrifty++;
//check if a client has connected
server.handleClient();
if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      s_time_date =digitalClockDisplay();
    }
  }
ultrasonicCalculation();
dailyCheck();  


}
////////////////////JavaScript//////////////////////////////////////
void buildJavascript(){
javaScript="<SCRIPT>\n";
  javaScript+="var xmlHttp=createXmlHttpObject();\n";

  javaScript+="function createXmlHttpObject(){\n";
  javaScript+=" if(window.XMLHttpRequest){\n";
  javaScript+="    xmlHttp=new XMLHttpRequest();\n";
  javaScript+=" }else{\n";
  javaScript+="    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
  javaScript+=" }\n";
  javaScript+=" return xmlHttp;\n";
  javaScript+="}\n";

  javaScript+="function process(){\n";
  javaScript+=" if(xmlHttp.readyState==0 || xmlHttp.readyState==4){\n";
  javaScript+="   xmlHttp.open('PUT','xml',true);\n";
  javaScript+="   xmlHttp.onreadystatechange=handleServerResponse;\n"; // no brackets?????
  javaScript+="   xmlHttp.send(null);\n";
  javaScript+=" }\n";
  javaScript+=" setTimeout('process()',1000);\n";
  javaScript+="}\n";
  
  javaScript+="function handleServerResponse(){\n";
  javaScript+=" if(xmlHttp.readyState==4 && xmlHttp.status==200){\n";
  javaScript+=" var  xmlResponse=xmlHttp.responseXML;\n";
  javaScript+="  var x = xmlResponse.getElementsByTagName('ultrasonic');\n";
  javaScript+="   document.getElementById('percentage_water_in_tank').innerHTML =x[0].childNodes[0].nodeValue;\n";
  javaScript+="  var x1 = xmlResponse.getElementsByTagName('timeAndDate');\n";
  javaScript+="   document.getElementById('current_time_date').innerHTML =x1[0].childNodes[0].nodeValue;\n";
  javaScript+="  var x2 = xmlResponse.getElementsByTagName('total_height');\n";
  javaScript+="   document.getElementById('total_height').innerHTML =x2[0].childNodes[0].nodeValue;\n";
  javaScript+="  var x3 = xmlResponse.getElementsByTagName('upper_level_percentage_water_in_tank');\n";
  javaScript+="   document.getElementById('upper_level_percentage_water_in_tank').innerHTML =x3[0].childNodes[0].nodeValue;\n";
  javaScript+="  var x4 = xmlResponse.getElementsByTagName('lower_level_percentage_water_in_tank');\n";
  javaScript+="   document.getElementById('lower_level_percentage_water_in_tank').innerHTML =x4[0].childNodes[0].nodeValue;\n";
  javaScript+="  var x5 = xmlResponse.getElementsByTagName('on_time');\n";
  javaScript+="   document.getElementById('on_time').innerHTML =x5[0].childNodes[0].nodeValue;\n";
   javaScript+="  var x6 = xmlResponse.getElementsByTagName('off_time');\n";
  javaScript+="   document.getElementById('off_time').innerHTML =x6[0].childNodes[0].nodeValue;\n";
  
  javaScript+=" }\n";
  javaScript+="}\n";
  //
  javaScript+="function turnOnServo(){\n";
  javaScript+=" document.getElementById('c_servo').innerHTML ='ON'; \n";
  javaScript+="}\n";
  javaScript+="function turnOffServo(){\n";
  javaScript+=" document.getElementById('c_servo').innerHTML ='OFF'; \n";
  javaScript+="}\n";
  //
  javaScript+="function turnOnMotor(){\n";
  javaScript+=" document.getElementById('c_motor').innerHTML ='ON'; \n";
  javaScript+="}\n";
  javaScript+="function turnOffMotor(){\n";
  javaScript+=" document.getElementById('c_motor').innerHTML ='OFF'; \n";
  javaScript+="}\n";
  javaScript+="</SCRIPT>\n";
}
////////////////////////BUILD webSite///////////////////////////////
void buildWebsite(){
   buildJavascript();
   webSite =  "<html>";
   webSite +=javaScript;
   webSite += "<head><title>Water Tank Control</title>";
   webSite += "<style>";
   webSite += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
   webSite += "</style>";
   webSite += "</head>";
   webSite += "<body onload='process()'>";
   webSite += "<h1><br>Water-Tank access:<div align='left'> Percentage Water in Tank:<a id='percentage_water_in_tank' ></a>% </div><div align='right' id='current_time_date'>Time will be shown here...<div></h1>";  
    
    String IPaddress = WiFi.localIP().toString();
    webSite += "<form action='http://"+IPaddress+"' method='GET'>";
    webSite +="<table border='0'><tr><td><div align='right'>Height of water-tank(in cm):</div></td> <td><input type='number' name='total_height' max='2000' required> &nbsp<a style='color:grey'>Current value: </a><a style='color:grey' id='total_height'></a><a style='color:grey'>cm</a></td></tr> ";
    webSite +="<tr><td><div align='right'>Max. permitted water(in %):</div></td> <td><input type='number' name='upper_level_percentage_water_in_tank' min='1' max='100' required>&nbsp<a style='color:grey'>Current value: </a><a style='color:grey' id='upper_level_percentage_water_in_tank'></a><a style='color:grey'>%</a></td></tr> ";
    webSite +="<tr><td><div align='right'>Min. permitted water (in%):</div></td> <td><input type='number' name='lower_level_percentage_water_in_tank' min='1' max='100' required>&nbsp<a style='color:grey'>Current value: </a><a style='color:grey' id='lower_level_percentage_water_in_tank'></a><a style='color:grey'>%</a></td></tr> ";
    webSite +="<tr><td><div align='right'>ON time:</div></td> <td><input type='time' name='on_time' required> &nbsp<a style='color:grey'>Current value: </a><a style='color:grey' id='on_time'></a></td></tr>";
    webSite +="<tr><td><div align='right'>OFF time:</div></td> <td><input type='time' name='off_time' required> &nbsp<a style='color:grey'>Current value: </a><a style='color:grey' id='off_time'></a></td></tr>";
    webSite +="<tr><td colspan='2'><center><input type='submit' value='Enter'></center></td> </tr></table>";
    
   /* webSite += "Height of water-tank(in cm):<input type='number' name='total_height' max='2000' required><br>";
    webSite += "Max. permitted water(in %):<input type='number' name='upper_level_percentage_water_in_tank' min='1' max='100' required><br>";
    webSite += "Min. permitted water (in%):<input type='number' name='lower_level_percentage_water_in_tank' min='1' max='100' required><br>";
    webSite +="ON time:<input type='time' name='on_time' required><br><br>";
    webSite +="OFF time:<input type='time' name='off_time' required><br><br>";
    webSite += "<input type='submit' value='Enter'>"; // omit <input type='submit' value='Enter'> for just 'enter'
    webSite += "</form><br><br>";*/
    //webSite +="<br> <br> <h1> Percentage Water in Tank:<a id='percentage_water_in_tank' ></a>%</h1>";
//    <form id='F2' action='LEDOFF'>
//    <input class='button' type='submit' value='LED OFF' >
//  </form>
    webSite +="<br><form action='servoOn'> Control output valve:&nbsp<input type='submit' value='Turn ON' > </form> ";
    webSite+="&nbsp <input type='submit' value='Turn OFF'> ";
    webSite+="&nbsp Currently:<a id='c_servo'> </a><br>";
    
    webSite +="<br>Control supply motor:&nbsp<input type='button' onclick='turnOnMotor()' value='Turn ON' > ";
    webSite+="&nbsp <input type='button' onclick='turnOffMotor()' value='Turn OFF'> ";
    webSite+="&nbspCurrently: <a id='c_motor'> </a> ";
    
    webSite += "</body>";
    webSite += "</html>";
}
/////////////////////////handleWebsite and XML///////////////////////////////// 
void handleWebsite() {
  buildWebsite();
  server.send(200, "text/html", webSite); // Send a response to the client asking for input
  if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      Serial.print(server.argName(i)); // Display the argument
      Argument_Name = server.argName(i);
      if (server.argName(i) == "total_height") {
        Serial.print(" total_height was: ");
        Serial.println(server.arg(i));
        total_height = server.arg(i).toFloat();
        }
      if (server.argName(i) == "upper_level_percentage_water_in_tank") {
        Serial.print(" upper_level_percentage_water_in_tank was: ");
        Serial.println(server.arg(i));
        upper_level_percentage_water_in_tank = server.arg(i).toInt();
      }
      if (server.argName(i) == "lower_level_percentage_water_in_tank") {
        Serial.print(" lower_level_percentage_water_in_tank was: ");
        Serial.println(server.arg(i));
        lower_level_percentage_water_in_tank = server.arg(i).toInt();
        // e.g. range_maximum = server.arg(i).toInt();   // use string.toInt()   if you wanted to convert the input to an integer number
        // e.g. range_maximum = server.arg(i).toFloat(); // use string.toFloat() if you wanted to convert the input to a floating point number
      }
      if (server.argName(i) == "on_time") {
        Serial.print(" on_time was: ");
        Serial.println(server.arg(i));
        on_time = server.arg(i);
      }
      if (server.argName(i) == "off_time") {
        Serial.print(" off_time was: ");
        Serial.println(server.arg(i));
        off_time = server.arg(i);
        // e.g. range_maximum = server.arg(i).toInt();   // use string.toInt()   if you wanted to convert the input to an integer number
        // e.g. range_maximum = server.arg(i).toFloat(); // use string.toFloat() if you wanted to convert the input to a floating point number
      }
      timeSplitting();
      
    }
  }
  else{
    total_height=75;
    upper_level_percentage_water_in_tank=80;
    lower_level_percentage_water_in_tank=20;
    on_time="08:20";
    off_time="10:30";
  }
}

void handleXML(){
  buildXML();
  server.send(200,"text/xml",XML);
}
//////////////XML//////////////////////
void buildXML(){
  XML="<?xml version='1.0'?>";
  XML+="<response>";
  XML+="<ultrasonic>";
  XML+=ultrasonicOutput();
  XML+="</ultrasonic>";
  XML+="<timeAndDate>";
  XML+=digitalClockDisplayOnlyWhen();
  XML+="</timeAndDate>";
  XML+="<form>";
  XML+="<total_height>"+String(total_height)+"</total_height>";
  XML+="<upper_level_percentage_water_in_tank>"+String(upper_level_percentage_water_in_tank)+"</upper_level_percentage_water_in_tank>";
  XML+="<lower_level_percentage_water_in_tank>"+String(lower_level_percentage_water_in_tank)+"</lower_level_percentage_water_in_tank>";
  XML+="<on_time>"+on_time+"</on_time>";
  XML+="<off_time>"+off_time+"</off_time>";
  XML+="</form>";

  
  XML+="</response>";
}
////////////////ultrasonic method////////////////////
String ultrasonicCalculation(){
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
}
String ultrasonicOutput(){
  return String(percentage_water_in_tank);
}
///////////////////////NTP-CODE and Clock display function///////////////////////////////
String digitalClockDisplay()
{
  // digital clock display of the time
  String s1_time_date ="Time:";
  if(hour()>12){
    int hours=hour()-12;
    s1_time_date+=""+String(hours);
  }
  else{
  if(hour()==00){
    int hours=hour()+12;
    s1_time_date+=""+String(hours);
  }
  else{
  s1_time_date+=""+String(hour()); 
  }
  }
  
  s1_time_date+=valDigits(minute());
  
  if(isAM()){
    s1_time_date+="AM";
  }
  else if(isPM()){
    s1_time_date+="PM";
  }
  
  s1_time_date+=" Date:";
  s1_time_date+=String(day());
  s1_time_date+=".";
  s1_time_date+=String(month());
  s1_time_date+=".";
  s1_time_date+=String(year());
   Serial.println(s1_time_date);
  return s1_time_date; 
 
}
String digitalClockDisplayOnlyWhen(){

  return s_time_date; // returns time and date when called by xml
}

String valDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  String val="";
  val+=":";
  if (digits < 10)
    val+="0";
  val+=digits;
  return val;
}
/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Received NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + (timeZone * SECS_PER_HOUR)+(timeZoneMin*60);
    }
  }
  Serial.println("No NTP Response :-(");
  getNtpTime();

  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
//////////////////Control servo and motor functions///////////////////////



////////////////////////////Daily function/////////////////////////////////
void dailyCheck(){
  if( motorstate=1){
  void turnOnServo(){
Serial.print("turning on servo");
}
else if(motorstate=0){
  void turnOffServo(){
Serial.print("turning off servo");
  }
}
  }
  if(hour()>=h_on && minute()>=m_on &&hour()<=h_off && minute()<=m_off){
//    Serial.println("motor works now my dear friend:");
//    Serial.print(on_time);
//    Serial.print(off_time);
   if(percentage_water_in_tank<upper_level_percentage_water_in_tank){
    motorstate=1;
    Serial.println("low water");
   }
  
   else if (percentage_water_in_tank>upper_level_percentage_water_in_tank){
    //turn off the water pump
   motorstate=0;
    Serial.println("tank full");
    //driver.waitPacketSent(); 
  } 
  
else {
    Serial.println("does not work now");
  }
  }
  motorOnOff();//calls the rf functions within  
}
void timeSplitting(){
  StringSplitter *splitter = new StringSplitter(on_time, ':', 2);  // new StringSplitter(string_to_split, delimiter, limit)
  h_on= (splitter->getItemAtIndex(0)).toInt();
  m_on=(splitter->getItemAtIndex(1)).toInt();
  StringSplitter *splitter1 = new StringSplitter(off_time, ':', 2);  // new StringSplitter(string_to_split, delimiter, limit)
  h_off= (splitter1->getItemAtIndex(0)).toInt();
  m_off=(splitter1->getItemAtIndex(1)).toInt();
}
void motorOnOff(){
    //checks if the motorstate variable has been set to what state and sends appropriate message via rf.  
    if(motorstate == 1){
    const char *msg = "1";
    driver.send((uint8_t *)msg, strlen(msg));
    Serial.println("1 has been sent");
    //driver.waitPacketSent();
    }
    else if (motorstate==0){
    const char *msg = "0";
    driver.send((uint8_t *)msg,strlen(msg));
    Serial.println("0 has been sent");
    //driver.waitPacketSent();
    
    }
 }


