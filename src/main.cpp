#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266mDNS.h>

const char *ssid = "SmartPot";
const char *password = "1234567891"; // at least 8 characters for AP
const boolean accessPoint = true;

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

#define USE_SERIAL Serial

// Sensor calibrate values and pump pin

int wet = 470;
int dry = 1024;
int pump = 5;
boolean pumpState = false;
int lastHum;

unsigned long int tsMessage;
unsigned long int tsPump;

int getHum() {
    int sensorValue = analogRead(A0);
    int humidity = map(sensorValue, wet, dry, 100, 0);
    if(humidity > 100) {
        humidity = 100;
    } else if(humidity < 0) {
        humidity = 0;
    }
    return humidity;
}

String getHumMSG() {
    int humidity = getHum();
    String msg = "";
    msg.concat(humidity);
    msg = 'h' + msg;
    return msg;
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch(type)
    {
    case WStype_DISCONNECTED:
        USE_SERIAL.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        String msg = getHumMSG();
        webSocket.sendTXT(num, msg);
        
    }
    break;
    case WStype_TEXT:

        USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

        String msg = reinterpret_cast<const char*>(payload);

        Serial.println(msg);
        char index = msg.charAt(0);
        msg.remove(0, 1);

        switch(index) {
            case 'p':
                if(pumpState == false) {
                    pumpState = true;
                    digitalWrite(pump, LOW);
                    webSocket.broadcastTXT("pTrue");
                    tsPump = millis();
                }
                break;
        }

        break;
    }
    
}

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  else if(filename.endsWith(".woff2")) return "font/woff2";
  return "text/plain";
}

void notFound(AsyncWebServerRequest *request) {

    // here comes some mambo-jambo to extract the filename from request->url()

    String url = request->url();
    Serial.println(url);

    String file = url.substring(url.lastIndexOf('/'));
    Serial.println(file);
    
    String contentType = getContentType(file);

    // ... and finally
    if(SPIFFS.exists(file)) {
        request->send(SPIFFS, file, contentType);
    } else {
        request->send(404, "text/plain", "The content you are looking for was not found.");
    }

}

void setup() {
    
    USE_SERIAL.begin(115200);

    pinMode(pump, OUTPUT);
    digitalWrite(pump, HIGH);

    if (!SPIFFS.begin())
    {
        USE_SERIAL.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // start Access Point
    WiFi.disconnect(true);
    if(accessPoint == true) {
        WiFi.softAP(ssid, password);
        USE_SERIAL.println(WiFi.softAPIP());
    } else {
        //Connect to Network
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }
        USE_SERIAL.println(WiFi.localIP());
    }

    // Route for root / web page

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server.onNotFound(notFound);

    if (!MDNS.begin("smartpot"))
    {
        USE_SERIAL.println("mDNS failed!");
    }
    else
    {
        USE_SERIAL.println("mDNS responder started!");
    }
    MDNS.addService("http", "tcp", 80);

    // Start server
    server.begin();

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop()
{
    MDNS.update();
    webSocket.loop();
    unsigned long int time = millis();
    if(pumpState == false) {
        if(tsMessage == NULL || tsMessage + 1000 < time) {
            String msg = getHumMSG();
            webSocket.broadcastTXT(msg);
            tsMessage = millis();

            lastHum = getHum();
        }
    } else {
        if(tsMessage == NULL || tsMessage + 300 < time) {
            String msg = getHumMSG();
            webSocket.broadcastTXT(msg);
            tsMessage = millis();

            lastHum = getHum();
        }
    }
    
    
    // pump control

    if(pumpState == false) {
        if(lastHum != NULL && lastHum < 50) {
            digitalWrite(pump, LOW);
            pumpState = true;
            tsPump = millis();
            webSocket.broadcastTXT("pTrue");
            String msg = getHumMSG();
            webSocket.broadcastTXT(msg);
        }
    }
    
    if(tsPump != NULL && pumpState == true) {
        if(tsPump + 2000 < time) {
            digitalWrite(pump, HIGH);
            pumpState = false;
            webSocket.broadcastTXT("pFalse");
            String msg = getHumMSG();
            webSocket.broadcastTXT(msg);
            tsPump = NULL;
            
        } 
    }
}
