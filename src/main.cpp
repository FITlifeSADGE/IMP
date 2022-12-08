/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <WiFi.h>
#include <Arduino.h>
#include <ESPmDNS.h>

// Replace with your network credentials
const char* ssid = "Kaprcka_2G";
const char* password = "bobina25";


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputRState = "off";
String outputBState = "off";
String outputGState = "off";

// Assign output variables to GPIO pins
const int outputR = 2;
const int outputB = 4;
const int outputG = 12;

int animation_on = 0;
int animation = 0;
int speed = 1;

int Rvalue = 0;
int Bvalue = 0;
int Gvalue = 0;

int Radd = 1;
int Badd = 0;
int Gadd = 0;

int switch_add = 0;

int prevR = 0, prevG = 0, prevB = 0; // all of the previous RGB values

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void RGB() {
  unsigned int rgbColour[3];
 // Start off with red.
 rgbColour[0] = 255;
 rgbColour[1] = 0;
 rgbColour[2] = 0;  
 // Choose the colours to increment and decrement.
 for (int decColour = 0; decColour < 3; decColour += 1) {
   int incColour = decColour == 2 ? 0 : decColour + 1;
   // cross-fade the two colours.
   for(int i = 0; i < 255; i += 1) {
    rgbColour[decColour] -= 1;
    rgbColour[incColour] += 1;
    analogWrite(outputR, rgbColour[0]);
    analogWrite(outputG, rgbColour[1]);
    analogWrite(outputB, rgbColour[2]);
    delay(10/speed);
   }
 }
}

void animations(void *pvParameter) {
  int counter = 0;
  while(1) {
    switch (animation) {
      case 1:
        analogWrite(outputR, 255);
        delay(300/speed);
        analogWrite(outputR, 0);
        delay(300/speed);
        analogWrite(outputB, 255);
        delay(300/speed);
        analogWrite(outputB, 0);
        delay(300/speed);
        analogWrite(outputG, 255);
        delay(300/speed);
        analogWrite(outputG, 0);
        delay(300/speed);
        break;
      case 2:
        if (Gadd) {
          if (switch_add) {
            Gvalue--;
            if (!Gvalue) {
              switch_add = 0;
              Gadd = 0;
              Radd = 1;
            }
          }
          else {
            Gvalue++;
          }
          if (Gvalue == 255) {
            switch_add = 1;
          }
        }
        else if (Badd) {
          if (switch_add) {
            Bvalue--;
            if (!Bvalue) {
              switch_add = 0;
              Badd = 0;
              Gadd = 1;
            }
          }
          else {
            Bvalue++;
          }
          if (Bvalue == 255) {
            switch_add = 1;
          }
        }
        else if (Radd) {
          if (switch_add) {
            Rvalue--;
            if (!Rvalue) {
              switch_add = 0;
              Radd = 0;
              Badd = 1;
            }
          }
          else {
            Rvalue++;
          }
          if (Rvalue == 255) {
            switch_add = 1;
          }
        }
        analogWrite(outputR, Rvalue);
        analogWrite(outputB, Bvalue);
        analogWrite(outputG, Gvalue);
        delay(10/speed);
        break;
      case 3:
        analogWrite(outputR, counter%255);
        analogWrite(outputB, (counter+100)%255);
        analogWrite(outputG, (counter+200)%255);
        delay(50/speed);
        break;
      case 4:
        RGB();
        delay(500/speed);
        break;
      default:
        digitalWrite(outputR, LOW);
        digitalWrite(outputB, LOW);
        digitalWrite(outputG, LOW);
        delay(200);
        break;
    }
    counter++;
  }
}

void setup() {
  xTaskCreate(&animations, "party_disco", 2048, NULL, 5, NULL);
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(outputR, OUTPUT);
  pinMode(outputB, OUTPUT);
  pinMode(outputG, OUTPUT);
  // Set outputs to LOW
  digitalWrite(outputR, LOW);
  digitalWrite(outputB, LOW);
  digitalWrite(outputG, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  //WiFi.softAP("ESP32", NULL);
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //Serial.println(WiFi.softAPIP());
  if(!MDNS.begin("esp32")) {
     Serial.println("Error starting mDNS");
     return;
  }
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /on") >= 0) {
              Serial.println("GPIO 2 on");
              outputRState = "on";
              outputBState = "on";
              outputGState = "on";
              analogWrite(outputR, 255);
              delay(200);
              analogWrite(outputR, 0);
            } else if (header.indexOf("GET /off") >= 0) {
              Serial.println("GPIO 2 off");
              outputRState = "off";
              outputBState = "off";
              outputGState = "off";
              animation_on = 0;
              animation = 0;
              speed = 1;
              analogWrite(outputR, 0);
              analogWrite(outputB, 0);
              analogWrite(outputG, 0);
              delay(200);
            } 
              else if (header.indexOf("GET /animation1") >= 0) {
                animation_on = 1;
                animation = 1;
                speed = 1;
              }
              else if (header.indexOf("GET /animation2") >= 0) {
                animation_on = 1;
                animation = 2;
                speed = 1;
              }
              else if (header.indexOf("GET /animation3") >= 0) {
                animation_on = 1;
                animation = 3;
                speed = 1;
              }
              else if (header.indexOf("GET /animation4") >= 0) {
                animation_on = 1;
                animation = 4;
                speed = 1;
              }
              else if (header.indexOf("GET /speed1") >= 0) {
                animation_on = 1;
                speed = 1;
              }
              else if (header.indexOf("GET /speed2") >= 0) {
                animation_on = 1;
                speed = 2;
              }
              else if (header.indexOf("GET /speed3") >= 0) {
                animation_on = 1;
                speed = 3;
              }
              else if (header.indexOf("GET /animation_off") >= 0) {
                Serial.println("animation OFF");
                animation_on = 0;
                animation = 0;
                speed = 1;
                analogWrite(outputR, 0);
                analogWrite(outputB, 0);
                analogWrite(outputG, 0);
              }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".animation_button > p {display: inline-block;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>RGB LED controls - xkapra00</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 2  
            client.println("<p>RGB LED state " + outputRState + "</p>");
            // If the outputRState is off, it displays the ON button       
            if (outputRState=="off") {
              client.println("<p><a href=\"/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/off\"><button class=\"button button2\">OFF</button></a></p>");
              client.println("<div class=\"animation_button\">");
              if (animation == 1) {
                client.println("<p><a href=\"/animation_off\"><button class=\"button button2\">Color switch</button></a></p>");
              }
              else {
                client.println("<p><a href=\"/animation1\"><button class=\"button button3\">Color switch</button></a></p>");
              }
              //client.println("<p><a href=\"/animation1\"><button class=\"button button3\">Color switch</button></a></p>");
              if (animation == 2) {
                client.println("<p><a href=\"/animation_off\"><button class=\"button button2\">Color fade</button></a></p>");
              }
              else {
                client.println("<p><a href=\"/animation2\"><button class=\"button button3\">Color fade</button></a></p>");
              }
              if (animation == 3) {
                client.println("<p><a href=\"/animation_off\"><button class=\"button button2\">Random color fade</button></a></p>");
              }
              else {
                client.println("<p><a href=\"/animation3\"><button class=\"button button3\">Random color fade</button></a></p>");
              }
              if (animation == 4) {
                client.println("<p><a href=\"/animation_off\"><button class=\"button button2\">RGB spectrum fade</button></a></p>");
              }
              else {
                client.println("<p><a href=\"/animation4\"><button class=\"button button3\">RGB spectrum fade</button></a></p>");
              }
              //client.println("<p><a href=\"/animation2\"><button class=\"button button3\">Color fade</button></a></p>");
              //client.println("<p><a href=\"/animation3\"><button class=\"button button3\">Random color fade</button></a></p>");
              //client.println("<p><a href=\"/animation4\"><button class=\"button button3\">RGB spectrum fade</button></a></p>");
              client.println("</div>");
              if (animation_on) {
                if (speed == 1) {
                  client.println("<p><a href=\"/speed1\"><button class=\"button button2\">Slow speed</button></a></p>");
                  client.println("<p><a href=\"/speed2\"><button class=\"button button3\">Medium speed</button></a></p>");
                  client.println("<p><a href=\"/speed3\"><button class=\"button button3\">Fast speed</button></a></p>");
                }
                if (speed == 2) {
                  client.println("<p><a href=\"/speed1\"><button class=\"button button3\">Slow speed</button></a></p>");
                  client.println("<p><a href=\"/speed2\"><button class=\"button button2\">Medium speed</button></a></p>");
                  client.println("<p><a href=\"/speed3\"><button class=\"button button3\">Fast speed</button></a></p>");
                }
                if (speed == 3) {
                  client.println("<p><a href=\"/speed1\"><button class=\"button button3\">Slow speed</button></a></p>");
                  client.println("<p><a href=\"/speed2\"><button class=\"button button3\">Medium speed</button></a></p>");
                  client.println("<p><a href=\"/speed3\"><button class=\"button button2\">Fast speed</button></a></p>");
                }
                //client.println("<p><a href=\"/speed1\"><button class=\"button button3\">Slow speed</button></a></p>");
                //client.println("<p><a href=\"/speed2\"><button class=\"button button3\">Medium speed</button></a></p>");
                //client.println("<p><a href=\"/speed3\"><button class=\"button button3\">Fast speed</button></a></p>");
              }

            } 
               
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}