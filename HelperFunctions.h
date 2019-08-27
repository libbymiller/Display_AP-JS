
/* Go to http:// 192.168.4.1 in a web browser
   connected to this access point to see it.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiManager.h>
//https://github.com/tzapu/WiFiManager

//#include <ESPmDNS.h>
// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESPmDNS/src/ESPmDNS.h

#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#define NAME "Oramics"

/*
// Below variables are used for Steinhart-Hart and associated temperature calcs
// Refer to // https://arduinodiy.wordpress.com/2015/11/10/measuring-temperature-with-ntc-the-steinhart-hart-formula/
#define            SERIESRESISTOR  150000    //  Reduced current and scale to 0 - 1 Volts
#define            NOMINAL_RESIST  10000     // @25C
#define            NOMINAL_TEMP    25        // See above
#define            BCOEFFICIENT    3950      // need to verify for Vishay 10K NTC
float              ADCvalue      = 0.0;      // Initialized values have no meaning
float              Resistance    = 0.0;      //       ditto
float              steinhart     = 0.0;      //       ditto
float              temperature   = 0.0;      //       ditto
int                temp_int      = 0;        //       ditto
int                correction    = 49;       // adjust readings to measured by accurate digital thermometer

// Below variables are general global variables
char               AP_NameChar[6];            // AP_NameString.length() + 1];
const char         WiFiAPPSK[]   = "";        // "put your password here before compiling"
String             AP_NameString = "Oramics";
String             temp_str;
*/

uint8_t            socketNumber;


ESP8266WebServer server(80);
WebSocketsServer webSocket(81);               // Create a Websocket server


void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch (type) { 
    case WStype_DISCONNECTED:
      // Reset the control for sending samples of ADC to idle to allow for web server to respond.
      Serial.printf("[%u] Disconnected!\n", num);
      yield();
      break;

    case WStype_CONNECTED: {                  // Braces required http://stackoverflow.com/questions/5685471/error-jump-to-case-label
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      yield();
      socketNumber = num;
      break;
      }

    case WStype_TEXT:
      if (payload[0] == '#') {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        yield();
      }
      break;

    case WStype_ERROR:
      Serial.printf("Error [%u] , %s\n", num, payload);
      yield();
  }
}


String getContentType(String filename) {
  yield();
  if (server.hasArg("download"))      return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html"))return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js"))  return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz"))  return "application/x-gzip";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}


bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);

  if (path.endsWith("/"))
  {
    path += "counter.html";
  }

  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  Serial.println("PathFile: " + pathWithGz);

  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {
    if (SPIFFS.exists(pathWithGz)) path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  yield();
  return false;
}


void setupWiFi()
{
/*
  WiFi.mode(WIFI_AP);
  // char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i = 0; i <AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);
  yield();
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
*/

    WiFiManager wifiManager;

    //reset settings - for testing
    //wifiManager.resetSettings();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    //wifiManager.setTimeout(120);

    //it starts an access point with the specified name
    //here  "Oramics"
    //and goes into a blocking loop awaiting configuration

    //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
    //WiFi.mode(WIFI_STA);
    
    if (!wifiManager.startConfigPortal(NAME)) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected");
    Serial.print("IP is ");
    Serial.println(WiFi.localIP());

    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

}


 
