/* Create a WiFi access point and provide a web server on it so show temperature. 
   Originally published by markingle on http://www.esp8266.com/viewtopic.php?p=47535
   Refactored and enhanced for Hackster.io by: M. Ray Burnette 20160620
   Arduino 1.6.9 on Linux Mint 64-bit version 17.3 compiled: 20160706 by Ray Burnette
    Sketch uses 284,865 bytes (27%) of program storage space. Maximum is 1,044,464 bytes.
    Global variables use 38,116 bytes (46%) of dynamic memory, leaving 43,836 bytes for local variables. Maximum is 81,920 bytes.
*/

#include <FS.h>
#include <WebSocketsServer.h>
#include "HelperFunctions.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_LIS3DH.h>

#include <Wire.h>
#include <SPI.h>

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

void setup() {
  Serial.begin(115200);

  // accelerometer
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("LIS3DH found!");
  
  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
  
  Serial.print("Range = "); Serial.print(2 << lis.getRange());  
  Serial.println("G");  
  
  delay(1000);

  //spiffs
  SPIFFS.begin();

  //AP
  Serial.println(); Serial.print("Configuring access point...");
  setupWiFi();
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: "); Serial.println(myIP);

  server.on("/", HTTP_GET, []() {
    handleFileRead("/");
  });

  server.onNotFound([]() {                          // Handle when user requests a file that does not exist
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  webSocket.begin();                                // start webSocket server
  webSocket.onEvent(webSocketEvent);                // callback function

  server.begin();
  Serial.println("HTTP server started");
  yield();
}

void loop() {
    static unsigned long l = 0;                     // only initialized once
    unsigned long t;                                // local var: type declaration at compile time
    
    t = millis();

    if((t - l) > 200) {                            // update every .2 second
        //analogSample(); yield();
        lis.read(); yield();
        sensors_event_t event; yield();
        lis.getEvent(&event); yield();
        webSocket.sendTXT(socketNumber, "x="+String(lis.x)+",y="+String(lis.y)+",z="+String(lis.z)+",xa="+String(event.acceleration.x)+",ya="+String(event.acceleration.y)+",za="+String(event.acceleration.z));
        l = t;                                      // typical runtime this IF{} == 300uS - 776uS measured
        yield();
    }

    server.handleClient();
    webSocket.loop();
}
