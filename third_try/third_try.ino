#include "Arduino.h"
#include <ESP8266WiFi.h>
//demo purposes
int openClosePin=5;//D1==>GPIO5


//declaring some variables for website creation 
String HTTP_req;
String code;
int serverPort=80;
WiFiServer server(serverPort);
//some variable for internet connection
const char* ssid="sebi";//the wifi network i am connecting to 
const char* pass="9172321736";//the password of the wifi netw.
const char* pass_sent="2345";//the pass. to protect the webpage
char codeOK='0';//assume start is 0
void setup() {
  //demo purpose
pinMode(openClosePin,INPUT);

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
}

void loop() {
  //check if a client has connected
  WiFiClient client =server.available();
  if(!client){
    return;
  }
  Serial.print("client available!");
  //return the response
  boolean currentLineIsBlank=true;
  codeOK='0';
  while(client.connected()){
    if(client.available()){//client data is availabel to read
      char c=client.read();//read 1 byte(char) from client
      HTTP_req +=c;//save the HTTP request 1 char at a time 
      // last line of client request is blank and ends with \n
      // respond to client only after last line received
      if (c == '\n' && currentLineIsBlank) {
        client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive");
        client.println();
        if (HTTP_req.indexOf("ajax_switch") > -1) {
          // read switch state and send appropriate paragraph text
          GetSwitchState(client);
          delay(0);
        }
        else {  // HTTP request for web page
          // send web page - contains JavaScript with AJAX calls
          client.print("<!DOCTYPE html>\r\n<html>\r\n<head>\r\n<title>Water tank</title>\r\n<meta name='viewport' content='width=device-width', initial-scale='1'>");
          client.print("<script>\r\nfunction GetSwitchState() {\r\nnocache = \"&nocache=\" + Math.random() * 1000000;\r\nvar request = new XMLHttpRequest();\r\nrequest.onreadystatechange = function() {\r\nif (this.readyState == 4) {\r\nif (this.status == 200) {\r\nif (this.responseText != null) {\r\ndocument.getElementById(\"switch_txt\").innerHTML = this.responseText;\r\n}}}}\r\nrequest.open(\"GET\", \"ajax_switch\" + nocache, true);\r\nrequest.send(null);\r\nsetTimeout('GetSwitchState()', 1000);\r\n}\r\n</script>\n");
          client.print("<script>\r\nfunction DoorActivate() {\r\nvar request = new XMLHttpRequest();\r\nrequest.open(\"GET\", \"door_activate\" + nocache, true);\r\nrequest.send(null);\r\n}\r\n</script>\n");
          client.print("</head>\r\n<body onload=\"GetSwitchState()\">\r\n<center><h1>ESP8266 WATER TANK</h1><hr>\r\n<div id=\"switch_txt\">\r\n</div>\r\n<br>\n");
          client.print("Input password to control PUMP AND SUPPLY VALVE.\r\n<br><br><form name=\"passcode_data\" onSubmit=\"GetCode()\"><input type=\"password\" name=\"password\" size='8' maxlength='4'>&nbsp;<input type=submit name=\"Submit\" value=\"Submit\" onClick=\"GetCode()\" style='height:22px; width:80px'></form><br><br>\n");
          if (HTTP_req.indexOf(pass_sent) > 0){
            GetCode();
          }
          if (codeOK == '0') {
            //client.print("<button type=\"button\" disabled style='height:50px; width:225px'>Push the Switch</button><br><br>\n");
  }
          if (codeOK == '1') {
           // client.print("<button type=\"button\" onclick=\"DoorActivate()\" style='height:50px; width:225px'>Push the Switch</button><br><br>\n");
            
            client.print("<form action=\"watertank_data\" method=\"get\"  >");
            client.print("Height of water-tank(in cm):<input type=\"number\" name=\"total_height\" max=\"2000\" required><br><br>");
            client.print("Max. permitted water(in %):<input type=\"number\" name=\"upper_level_percentage_water_in_tank\" min=\"1\" max=\"100\" required><br><br>");
            client.print("Min. permitted water (in%):<input type=\"number\" name=\"lower_percentage_water_in_tank\" min=\"1\" max=\"100\" required><br><br>");
           
            client.print("<input type=submit value=\"Submit Data\" ></form>");
          }
          if (HTTP_req.indexOf("door_activate") > -1) {
            // read switch state and send appropriate paragraph text
            DoorActivate();
          }
          client.print("</body>\r\n</html>\n");
          delay(0);
          }
           // display received HTTP request on serial port
        Serial.println(HTTP_req);
        HTTP_req = "";            // finished with request, empty string
        break;
        }
        if (c == '\n') {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // a text character was received from client
          currentLineIsBlank = false;
        }
    }// end if (client.available())
  }                                                           // end while (client.connected())
  delay(1);      // give the web browser time to receive the data
  client.stop(); // close the connection
  delay(0);
}
// send the state of the switch to the web browser
void GetSwitchState(WiFiClient cl) {
    if (digitalRead(openClosePin)) {
      cl.println("<p>Switch is currently: <span style='background-color:#FF0000; font-size:18pt'>OFF</span></p>");
    }
    else {
      cl.println("<p>Switch is currently: <span style='background-color:#00FF00; font-size:18pt'>ON</span></p>");
    }
  }
  void GetCode() {
      codeOK='1';
  }
  void DoorActivate() {
    if (digitalRead(openClosePin)) digitalWrite(openClosePin, LOW);
    else digitalWrite(openClosePin, HIGH);
    codeOK='0';
  }
