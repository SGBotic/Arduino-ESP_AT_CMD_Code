/* This sketch is to create a websever in Arduino Uno and allow user to control 
 *  its LED via web browser.
    - Download the sketch to the Arduino board.
    - Open serial monitor. Set the baudrate to 19200. Get the IP address.
    - Open web browser and key in the IP address to connect to this web server.
    - click the button on the brawser to turn on/off the LED

    **** replace SSID and password in line 36 ******
*/


#include <SoftwareSerial.h>

#define TIMEOUT 5000 // mS
#define LED 13

SoftwareSerial mySerial(7, 6); // RX, TX

int ledState = 0;

void setup()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  ledState = 0;

  Serial.begin(19200);
  mySerial.begin(19200);
  Serial.println("Reset ESP....");
  SendCommand("AT+RST", "Ready"); //reset ESP
  delay(1000);
  SendCommand("AT+CWMODE=1", "OK"); //1=station mode

  //connect to wifi network
  //replace mySSID and myPassword with your wifi setting
  SendCommand("AT+CWJAP=\"mySSID\",\"myPassword\"", "OK");  //connect to wifi network
  
  SendCommand("AT+CIFSR", "OK");   //get ip address
  SendCommand("AT+CIPMUX=1", "OK");  //1=multiple connect, 0=single connection
  SendCommand("AT+CIPSERVER=1,80", "OK"); //1=create server, port=80
}

void loop() {
  String inString;

  while (mySerial.available()) {
    char findString[] = "+IPD,";
    if (mySerial.find(findString))
    {
      delay(1000);

      //this web server accept multiple connections. So it is important to identify the incoming request 
      //in order to return the result in the later part of this program.
      int connectionId = mySerial.read() - 48; // result is in ASCII. Subtract 48 (decimal number of ASCII '0')
                                                // to get integer value.
     
      Serial.print("Receive connection from: ");
      Serial.println(connectionId);

      inString = mySerial.readString();

      if (inString.indexOf("LED_ON") != -1) {
        ledState = 1;
        digitalWrite(LED, HIGH); //turn on LED
        Serial.println("LED ON");
      }

      if (inString.indexOf("LED_OFF") != -1) {
        ledState = 0;
        digitalWrite(LED, LOW);  //turn off LED
        Serial.println("LED OFF");
      }

      sendHtml(connectionId, "<!DOCTYPE html><html>");
      sendHtml(connectionId, "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
      sendHtml(connectionId, "<link rel=\"icon\" href=\"data:,\">");
      sendHtml(connectionId, "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
      sendHtml(connectionId, ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
      sendHtml(connectionId, "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
      sendHtml(connectionId, ".button2 {background-color: #77878A;}</style></head>");
      sendHtml(connectionId, "<h1>ESP8266 Web Server</h1>");
      if (ledState == 0) {
        sendHtml(connectionId, "<p>LED State = OFF</p>");
        sendHtml(connectionId, "<p><a href=\"/LED_ON\"><button class=\"button\">ON</button></a></p>");
      } else {
        sendHtml(connectionId, "<p>LED State = ON</p>");
        sendHtml(connectionId, "<p><a href=\"/LED_OFF\"><button class=\"button\">OFF</button></a></p>");
      }

      String closeCommand = "AT+CIPCLOSE=";
      closeCommand += connectionId; // append connection id
      SendCommand(closeCommand, "OK");
    }
  }
}


void sendHtml(int id, String htmlString)
{
  String cipCmd = "AT+CIPSEND=";
  cipCmd += id;
  cipCmd += ",";
  cipCmd += htmlString.length();
  SendCommand(cipCmd, "OK");
  delay(250);
  SendCommand(htmlString, "OK");
  delay(250);
}

boolean SendCommand(String cmd, String ack) {
  mySerial.println(cmd); // Send "AT+" command to module
  if (!echoFind(ack)) // timed out waiting for ack string
    return true; // ack blank or ack found
}

boolean echoFind(String keyword) {
  byte current_char = 0;
  byte keyword_length = keyword.length();
  long deadline = millis() + TIMEOUT;
  while (millis() < deadline) {
    if (mySerial.available()) {
      char ch = mySerial.read();
      Serial.write(ch);
      if (ch == keyword[current_char])
        if (++current_char == keyword_length) {
          Serial.println();
          return true;
        }
    }
  }
  return false; // Timed out
}
