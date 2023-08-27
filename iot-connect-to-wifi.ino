#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

struct WiFiCredentials {
  char ssid[32];
  char password[64];
};

WiFiCredentials storedCredentials;

const char* hotspotSSID = "SetupHotspot";
const char* hotspotPassword = "password123";

ESP8266WebServer server(80);

bool deviceSettled = false;

void setup() {
  Serial.begin(115200);

  EEPROM.begin(sizeof(WiFiCredentials));
  EEPROM.get(0, storedCredentials);

  // Start hotspot for receiving new credentials
  WiFi.softAP(hotspotSSID, hotspotPassword);

  Serial.println();
  Serial.print("Hotspot IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_POST, handlePost);
  server.on("/", HTTP_DELETE, deleteCredentials);  // Handle DELETE request
  server.begin();

  // Try to connect to the saved Wi-Fi network
  if (strlen(storedCredentials.ssid) > 0 && strlen(storedCredentials.password) > 0) {
    WiFi.begin(storedCredentials.ssid, storedCredentials.password);
    Serial.println("Connecting to saved Wi-Fi...");
  }
}

void loop() {
  server.handleClient();

  // Check if Wi-Fi is connected
  if (WiFi.status() == WL_CONNECTED) {
    if (!deviceSettled) {
      Serial.println("Connected to Wi-Fi.");
      WiFi.softAPdisconnect(true);  // Stop the hotspot
      deviceSettled = true;
    }
    // Your main loop logic here
  } else {
    deviceSettled = false;
  }
}

void handlePost() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  // Attempt to connect to the new Wi-Fi network using received credentials
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long connectStartTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - connectStartTime < 15000) {
    delay(1000);
    server.handleClient();  // Keep handling requests while connecting
  }

  if (WiFi.status() == WL_CONNECTED) {
    server.send(200, "text/plain", "Wi-Fi credentials received and connected.");

    // Store the new credentials in EEPROM
    strncpy(storedCredentials.ssid, ssid.c_str(), sizeof(storedCredentials.ssid));
    strncpy(storedCredentials.password, password.c_str(), sizeof(storedCredentials.password));
    EEPROM.put(0, storedCredentials);
    EEPROM.commit();
    Serial.println("Connected to new Wi-Fi and saved credentials.");
    // Successfully connected, so stop the hotspot
    WiFi.softAPdisconnect(true);
  } else {
    // Connection failed, keep the hotspot running
    server.send(400, "text/plain", "Wi-Fi connection failed. Please check the credentials.");
    Serial.println("Failed to connect to new Wi-Fi.");
  }
}

void deleteCredentials() {
  Serial.println("Received DELETE request. Deleting credentials...");

  // Clear the stored credentials in EEPROM
  memset(&storedCredentials, 0, sizeof(storedCredentials));
  EEPROM.put(0, storedCredentials);
  EEPROM.commit();

  // Disconnect from Wi-Fi if connected
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Disconnecting from Wi-Fi...");
    WiFi.disconnect();
  }

  Serial.println("Restarting ESP8266...");
  // Restart the ESP8266
  ESP.restart();
}
