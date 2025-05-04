#include <Arduino.h>
#include "DHT.h"
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <WiFiClientSecure.h>

// DHT Sensor Config
#define DHTTYPE DHT22
#define DHTPIN1 4
#define DHTPIN2 5
#define DHTPIN3 18
#define DHTPIN4 19
#define DHTPIN5 21

// DHT Sensor Objects
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);
DHT dht4(DHTPIN4, DHTTYPE);
DHT dht5(DHTPIN5, DHTTYPE);

// Wi-Fi
#define WIFI_SSID "Your_wifi_SSID"
#define WIFI_PASSWORD "Your_wifi_password"

// Firebase
#define FIREBASE_HOST "Your_project_id.firebaseio.com"
#define FIREBASE_AUTH "Your_firebase_auth_token"

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Telegram
#define BOT_TOKEN "Your_bot_token"
#define CHAT_ID "Your_chat_id"

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);  // GMT+5:30

// Scheduling
int lastScheduledMinute = -1;
String sectionId = "Section_01"; // Change this to your section ID if using multiple sections

// Function to send Telegram message
void sendTelegramMessage(String message) {
  WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect("api.telegram.org", 443)) return;
  String url = "/bot" + String(BOT_TOKEN) + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + message;
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: api.telegram.org\r\n" + "Connection: close\r\n\r\n");
  while (client.connected() || client.available()) {
    while (client.available()) client.read();
  }
  client.stop();
}

void setup() {
  Serial.begin(115200);
  dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();
  dht5.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  int retry = 0;

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED && retry < 15) {
    delay(1000);
    retry++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    // WiFi connected
    sendTelegramMessage("✅ " + sectionId + " Connected to WiFi");
  } else {
    // WiFi failed
    sendTelegramMessage("❌ ESP32 failed to connect to WiFi");
    return;
  }
  timeClient.begin();
  timeClient.update();
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// Function to read and upload sensor data
void readAndUploadSensor(DHT& dht, const String& sensorName) {
  float h = NAN, t = NAN;
  for (int i = 0; i < 3 && (isnan(h) || isnan(t)); i++) {
    h = dht.readHumidity();
    t = dht.readTemperature();
    delay(1000);
  }
  if (isnan(h) || isnan(t)) {
    sendTelegramMessage("[" + sectionId + "] ❌ " + sensorName + " read failed");
    return;
  }

// Upload to Firebase
  timeClient.update();
  time_t epoch = timeClient.getEpochTime();
  struct tm* tmStruct = gmtime(&epoch);
  char timeStr[20];
  snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d_%02d:%02d:%02d",
           tmStruct->tm_year + 1900, tmStruct->tm_mon + 1, tmStruct->tm_mday,
           tmStruct->tm_hour, tmStruct->tm_min, tmStruct->tm_sec);
  String timestamp(timeStr);
  String path = "/" + sectionId + "/" + sensorName + "/" + timestamp;

  if (!Firebase.setFloat(firebaseData, path + "/Humidity", h))
    sendTelegramMessage("[" + sectionId + "] ❌ " + sensorName + " Humidity upload failed");
  if (!Firebase.setFloat(firebaseData, path + "/Temperature", t))
    sendTelegramMessage("[" + sectionId + "] ❌ " + sensorName + " Temperature upload failed");
}

// Main loop
void loop() {
  timeClient.update();
  int currentMinute = timeClient.getMinutes();
  if (currentMinute % 10 == 0 && currentMinute != lastScheduledMinute) {
    lastScheduledMinute = currentMinute;
    readAndUploadSensor(dht1, "Sensor1");
    readAndUploadSensor(dht2, "Sensor2");
    readAndUploadSensor(dht3, "Sensor3");
    readAndUploadSensor(dht4, "Sensor4");
    readAndUploadSensor(dht5, "Sensor5");
  }
  delay(5000);
}
