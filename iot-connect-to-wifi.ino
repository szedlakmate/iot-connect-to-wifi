#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "SetupHotspot";
const char* password = "password123";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.print("Hotspot IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_POST, handlePost);
  server.begin();
}

void loop() {
  server.handleClient();
  // Add other loop functionality here if needed
}

void handlePost() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  // Connect to the specified Wi-Fi network
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  server.send(200, "text/plain", "Wi-Fi credentials received and connected to the network.");

  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP

  // Stop the hotspot
  WiFi.softAPdisconnect(true);
}
