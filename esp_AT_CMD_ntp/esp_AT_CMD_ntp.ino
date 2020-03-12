/* 
 *  This sketch is to request and receive time from a NTP (network time protocol) server
 *  
 *  **** replace SSID and password in line 29 ******
*/



#include <SoftwareSerial.h>

#define TIMEOUT 5000 // mS

// Last Packet Send Time (MilliSecond)
unsigned long lastSendPacketTime = 0;

SoftwareSerial mySerial(7, 6); // RX, TX

void setup()
{
  Serial.begin(19200);
  mySerial.begin(19200);
  Serial.println("Reset ESP....");
  SendCommand("AT+RST", "Ready"); //reset ESP
  delay(1000);
  SendCommand("AT+CWMODE=1", "OK"); //1=station mode

  //connect to wifi network
  //replace mySSID and myPassword with your wifi setting
  SendCommand("AT+CWJAP=\"mySSID\",\"myPassword\"", "OK");  //connect to wifi network
  
  //SendCommand("AT+CIFSR", "OK");   //get ip address
  //SendCommand("AT+CIPMUX=1", "OK");  //1=multiple connect, 0=single connection

  //Set timezone and connect to multiple NTP servers
  SendCommand("AT+CIPSNTPCFG=1,8,\"0.sg.pool.ntp.org\",\"cn.ntp.org.cn\",\"us.pool.ntp.org\"", "OK");
}

void loop() {
  String inString;
  String timeString;

  //send NTP request - one second interval
  long now = millis();
  if (now - lastSendPacketTime > 1000) { // One second has elapsed
    lastSendPacketTime = now;
    mySerial.println("AT+CIPSNTPTIME?");
  }
  
  while (mySerial.available()) {
    char findString[] = "+CIPSNTPTIME:";
    if (mySerial.find(findString)) 
    {
      inString = mySerial.readString();
    }
    Serial.println(" ");
    //Serial.println(inString);
    inString = splitString(inString, '\r', 0);  //remove 'OK' from string
    Serial.println(inString);
    Serial.println("Day: " + splitString(inString, ' ', 0));
    Serial.println("Month: " + splitString(inString, ' ', 1));
   
    if (splitString(inString, ' ', 2) == NULL) //date < 10
    {
      Serial.println("Date: " + splitString(inString, ' ', 3));
      Serial.println("Yeare: " + splitString(inString, ' ', 5));
      timeString = splitString(inString, ' ', 4);
    } else
    {
      Serial.println("Date: " + splitString(inString, ' ', 3));
      Serial.println("Yeare: " + splitString(inString, ' ', 5));
      timeString = splitString(inString, ' ', 4);
    }
    Serial.println("Hour: " + splitString(timeString, ':', 0));
    Serial.println("Minute: " + splitString(timeString, ':', 1));
    Serial.println("Second: " + splitString(timeString, ':', 2));
  }

}

boolean SendCommand(String cmd, String ack) {
  mySerial.println(cmd); // Send "AT+" command to module
  if (!echoFind(ack))    // timed out waiting for ack string
    return true;         // ack blank or ack found
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

String splitString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
