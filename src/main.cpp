#include <ESP8266WiFi.h>

//needed for library
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#include <Ticker.h>
Ticker ticker;

#include "secrets.h"

const int ledPin = LED_BUILTIN;

// --- IFTTT integration
String name = "Rogue_One";
String event = "call_home";

const char *IFTTT_URL = "maker.ifttt.com";
// auth key in secrets.h
const uint16_t httpsPort = 443;

void tick()
{
  //toggle state
  int state = digitalRead(ledPin); // get the current state of GPIO1 pin
  digitalWrite(ledPin, !state);    // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println(WiFi.softAPIP());

  //entered config mode, make led toggle faster
  ticker.attach(0.1, tick);
}

void call_home()
{
  WiFiClientSecure s_client;

  if (!s_client.connect(IFTTT_URL, httpsPort))
  {
    Serial.println("Secure Connection failed");
    return;
  }

  String url = "/trigger/" + event + "/with/key/" + secrets::auth_key;

  IPAddress ip = WiFi.localIP();
  String source = "SSID (" + WiFi.SSID() + ") with IP (" + String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) + ")";

  String postData = "{\"value1\":\"" + name + "\",\"value2\":\"" + source + "\",\"value3\":\"" + "Test_Data" + "\"}";
  Serial.println("requesting URL: ");
  Serial.println(url);
  Serial.println("with parameters: ");
  Serial.println(postData);

  s_client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + IFTTT_URL + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Accept: */*\r\n" +
                 "Content-Length: " + String(postData.length()) + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: close\r\n" +
                 "\r\n" +
                 postData + "\r\n");

  Serial.println("Request sent");
  Serial.println("Reply:");
  while (s_client.connected())
  {
    String line = s_client.readStringUntil('\n');
    Serial.println(line);
  }
}

void setup()
{
  Serial.begin(9600);
  delay(10);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, !LOW);

  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.4, tick);

  WiFiManager wifiManager;

  wifiManager.resetSettings();

  wifiManager.setAPCallback(configModeCallback);
  //set custom ip for portal
  wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));

  if (!wifiManager.autoConnect("ESP_AP"))
  {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();
  //keep LED on
  digitalWrite(LED_BUILTIN, LOW);

  call_home();
}

void loop()
{ /*
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {  //no client
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request

  int value = LOW;
  if (request.indexOf("/LED=ON") != -1) {
    digitalWrite(ledPin, !HIGH);
    value = HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1){
    digitalWrite(ledPin, !LOW);
    value = LOW;
  }

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("Led pin is now: ");

  if(value == HIGH) {
    client.print("On");
  } else {
    client.print("Off");
  }
  client.println("<br><br>");
  client.println("Click <a href=\"/LED=ON\">here</a> turn the LED on pin 5 ON<br>");
  client.println("Click <a href=\"/LED=OFF\">here</a> turn the LED on pin 5 OFF<br>");
  client.println("</html>");

  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");
*/
}
