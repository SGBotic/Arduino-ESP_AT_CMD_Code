
/* This sketch is to create a MQTT client in Arduino Uno and allow user to publish
    and subscribe to MQTT topic
    - Sign up / log in to https://www.cloudmqtt.com/
    - Take note of Server name, User, Password and Port (we are not using the SSL port)
    - Download the sketch to the Arduino board.
    - Open serial monitor. Set the baudrate to 19200.
    - In cloudmqtt.com, go to "Connections". You should see your IP if successfully connected.
    - In cloudmqtt.com, go to "WEBSOCKET UI". In "Send Message", set "Topic" to "LED" and "Message" to "ON", click "Send" button
    - The LED on your Arduino board should turns on.
    - Set "Message" to "OFF", click "Send" button to turn off the LED
    - The sketch send the value of A0 to server every 5 seconds

    IMPORTANT:
    **** replace Wifi SSID and password in line 48 ******
    **** replace cloudmqtt username and password in line 57 ******
    **** replace cloudmqtt server and port in line 62 ******
*/

#include <SoftwareSerial.h>

#define TIMEOUT 5000 // mS
#define LED 13

SoftwareSerial mySerial(7, 6); // RX, TX
const int button = 11;
int button_state = 0;

int interval = 3000; //3 seconds
unsigned long time_now = 0;
int sensorPin = A0;    // select the input pin for the potentiometer

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  pinMode(button, INPUT);
  digitalWrite(LED, LOW);

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




  //cloudmqtt.com user setting
  //replace cloudmqtt_username and cloudmqtt_password with your cloudmqtt setting
  String userCfg = "AT+MQTTUSERCFG=0,1,\"ESP\",\"cloudmqtt_username\",\"cloudmqtt_password\",0,0,\"\"";
  SendCommand(userCfg, "ok");

  //cloudmqtt.com server and port setting
  //replace cloudmqtt_server and cloudmqtt_server_port with your cloudmqtt setting
  String conn = "AT+MQTTCONN=0,\"cloudmqtt_server\",cloudmqtt_server_port,0";
  SendCommand(conn, "ok");

  String sub = "AT+MQTTSUB=0,\"LED\",0";
  SendCommand(sub, "ok");
  //SendCommand("ATE0", "OK"); //Turn Echo off

  time_now = millis();

}

void loop() {
  String inString;
  String temp;

  while (mySerial.available() > 0) {

    inString = mySerial.readString();
    Serial.println(inString);

    if (inString.indexOf("LED") != -1) {  //check topic
      if (inString.indexOf("ON") != -1) { //check message
        digitalWrite(LED, HIGH);
        Serial.println("LED ON");
      } else if (inString.indexOf("OFF") != -1) {
        digitalWrite(LED, LOW);
        Serial.println("LED OFF");
      }
    }
  }


  if (millis() > time_now + interval)  //publish to server every 5 seconds
  {
    int sensorData = analogRead(sensorPin);

    String pub = "AT+MQTTPUB=0,\"" + String(sensorData) + "\",\"Sensor\",1,0";
    mySerial.println(pub);
    mySerial.flush();  //clear mySerial buffer
    time_now = millis();
  }
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
