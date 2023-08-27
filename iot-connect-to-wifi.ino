#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

const char* ssid = "SetupHotspot";
const char* password = "password123";

ESP8266WebServer server(80);

struct WiFiCredentials {
  char ssid[32];
  char password[64];
};

WiFiCredentials storedCredentials;

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.print("Hotspot IP address: ");
  Serial.println(WiFi.softAPIP());

  EEPROM.begin(sizeof(WiFiCredentials));
  EEPROM.get(0, storedCredentials);

  if (strlen(storedCredentials.ssid) > 0 && strlen(storedCredentials.password) > 0) {
    // Try to connect to the saved Wi-Fi network
    WiFi.begin(storedCredentials.ssid, storedCredentials.password);
    Serial.println("Connecting to saved Wi-Fi...");
    unsigned long connectStartTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - connectStartTime < 60000) {
      delay(1000);
      Serial.println("Connecting to Wi-Fi...");
    }

    if (WiFi.status() == WL_CONNECTED) {
      // Successfully connected, so stop the hotspot
      WiFi.softAPdisconnect(true);
      Serial.println("Connected to Wi-Fi.");
    } else {
      // Could not connect in time, start hotspot
      Serial.println("Could not connect to saved Wi-Fi. Starting hotspot.");
    }
  } else {
    // No saved credentials, start hotspot
    Serial.println("No saved Wi-Fi credentials. Starting hotspot.");
  }

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
  WiFi.begin(storedCredentials.ssid, storedCredentials.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  server.send(200, "text/plain", "Wi-Fi credentials received and connected to the network.");

  // Store the credentials in EEPROM
  strncpy(storedCredentials.ssid, ssid.c_str(), sizeof(storedCredentials.ssid));
  strncpy(storedCredentials.password, password.c_str(), sizeof(storedCredentials.password));
  EEPROM.put(0, storedCredentials);
  EEPROM.commit();

  // Stop the hotspot
  WiFi.softAPdisconnect(true);
}
