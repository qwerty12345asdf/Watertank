#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h> 

//Time variables
String on_time,off_time; 

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
int lower_percentage_water_in_tank;//expose to the internet
float current_height;
const int echoPin = 16;  //D0
const int trigPin = 5;  //D1

//Variables to control the servo
Servo myservo;
int servo_control_pin=4;//GIPO4 OR D2 pin


//some variable for internet connection
const char* ssid="FD 47";//the wifi network i am connecting to 
const char* pass="123456789012";//the password of the wifi netw.


///////////////////////SETUP//////////////////////////////////////////////////
void setup() {
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
  //Start the server
  server.begin();
  Serial.println("Server started");
  //print the ip address
  Serial.println("Use this url to connect:");
  Serial.println("http://");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.print(serverPort);
  Serial.print("/");
  delay(1000);
  // Next define what the server should do when a client connects
  server.on("/", handleWebsite); // The client connected with no arguments e.g. http:192.160.0.40/
  server.on("/xml",handleXML);
  //server.on("/result", ShowClientResponse);
 }
/////////////////////////LOOP////////////////////////////////////////
void loop() {
  //check if a client has connected
server.handleClient();


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
  javaScript+="   xmlResponse=xmlHttp.responseXML;\n";
  javaScript+="   xmldoc = xmlResponse.getElementsByTagName('response');\n";
  javaScript+="   message = xmldoc[0].firstChild.nodeValue;\n";
  javaScript+="   document.getElementById('percentage_water_in_tank').innerHTML=message;\n";
  javaScript+=" }\n";
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
   
    webSite += "<h1><br>Water-Tank access:</h1>";  
    webSite +="<div>Time will be shown here...<div>";
    String IPaddress = WiFi.localIP().toString();
    webSite += "<form action='http://"+IPaddress+"' method='POST'>";
    webSite +="<table border='0'><tr><td><div align='right'>Height of water-tank(in cm):</div></td> <td><input type='number' name='total_height' max='2000' required></td></tr> ";
    webSite +="<tr><td><div align='right'>Max. permitted water(in %):</div></td> <td><input type='number' name='upper_level_percentage_water_in_tank' min='1' max='100' required></td></tr> ";
    webSite +="<tr><td><div align='right'>Min. permitted water (in%):</div></td> <td><input type='number' name='lower_percentage_water_in_tank' min='1' max='100' required></td></tr> ";
    webSite +="<tr><td><div align='right'>ON time:</div></td> <td><input type='time' name='on_time' required></td></tr>";
    webSite +="<tr><td><div align='right'>OFF time:</div></td> <td><input type='time' name='off_time' required></td></tr>";
    webSite +="<tr><td colspan='2'><center><input type='submit' value='Enter'></center></td> </tr></table>";
    
   /* webSite += "Height of water-tank(in cm):<input type='number' name='total_height' max='2000' required><br>";
    webSite += "Max. permitted water(in %):<input type='number' name='upper_level_percentage_water_in_tank' min='1' max='100' required><br>";
    webSite += "Min. permitted water (in%):<input type='number' name='lower_percentage_water_in_tank' min='1' max='100' required><br>";
    webSite +="ON time:<input type='time' name='on_time' required><br><br>";
    webSite +="OFF time:<input type='time' name='off_time' required><br><br>";
    webSite += "<input type='submit' value='Enter'>"; // omit <input type='submit' value='Enter'> for just 'enter'
    webSite += "</form><br><br>";*/
    webSite +="<br> <br> <h1> Percentage Water in Tank:<a id='percentage_water_in_tank' ></a>%</h1>";
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
      if (server.argName(i) == "lower_percentage_water_in_tank") {
        Serial.print(" lower_percentage_water_in_tank was: ");
        Serial.println(server.arg(i));
        lower_percentage_water_in_tank = server.arg(i).toInt();
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
    }
  }
  else{
    total_height=75;
    upper_level_percentage_water_in_tank=80;
    lower_percentage_water_in_tank=20;
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
  XML+=responses();
  XML+="</response>";
}
////////////////ultrasonic method////////////////////
String responses(){
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
  
  return String(percentage_water_in_tank);
}

