# ESP32 DHT22 Firebase Logger with Telegram Alerts

This project reads temperature and humidity data from multiple DHT22 sensors connected to an ESP32, logs the data to Firebase Realtime Database, and sends alert messages via Telegram. Readings are aligned precisely to local time at 10-minute intervals (e.g., 10:00, 10:10, etc.).

---

## 🧰 Components Required

- ESP32 Development Board
- 5x DHT22 Sensors
- Jumper wires and breadboard
- Internet-connected Wi-Fi network

---

## ⚙️ Features

- Reads temperature and humidity from 5 sensors.
- Sends data to Firebase Realtime Database.
- Sends messages to Telegram if reading/upload fails.
- Syncs local time with NTP and logs data every 10 minutes at fixed times (e.g., 10:00, 10:10...).

---

## 🔧 Setup Instructions

### 1. 🔌 Circuit Connections

| DHT Sensor | ESP32 GPIO |
| ---------- | ---------- |
| DHT22 #1   | GPIO 4     |
| DHT22 #2   | GPIO 5     |
| DHT22 #3   | GPIO 18    |
| DHT22 #4   | GPIO 19    |
| DHT22 #5   | GPIO 21    |

> Connect all DHT22 VCC pins to 3.3V, GND to GND, and DATA to corresponding GPIO pins. Add a 10K pull-up resistor between DATA and VCC of each sensor.

---

### 2. 🧑‍💻 Firebase Configuration

1. Go to [Firebase Console](https://console.firebase.google.com/).
2. Create a project and enable **Realtime Database**.
3. In the Realtime Database:

   - Click "Rules" and set to:
     ```json
     {
       "rules": {
         ".read": true,
         ".write": true
       }
     }
     ```
   - (Only for testing; secure it properly for production.)

4. In **Project Settings > Service accounts > Database secrets**, copy the **Legacy Token**.
5. Note down your database URL:  
   Format: `https://your-project-id.firebaseio.com/`

---

### 3. 🤖 Create a Telegram Bot

1. Open Telegram and search for [@BotFather](https://t.me/BotFather).
2. Type `/start`, then `/newbot` and follow prompts.
3. Copy the **bot token** you receive (e.g., `123456:ABC-DEF...`).
4. Get your chat ID:
   - Start a conversation with your bot.
   - Visit this URL in browser:
     ```
     https://api.telegram.org/bot<YourBotToken>/getUpdates
     ```
   - Your chat ID will appear in the response JSON under `"chat":{"id":...}`.

---

### 4. 🔨 Arduino IDE Setup

1. Install **ESP32 board** in Arduino IDE:

   - File > Preferences > Add URL:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Then go to Tools > Board Manager and install **esp32**.

2. Install required libraries via Library Manager:
   - `DHT sensor library`
   - `FirebaseESP32` by Mobizt
   - `NTPClient`
   - `Time` by Michael Margolis
   - `WiFiClientSecure`

---

### 5. 📝 Configure the Code

In the code, update these macros with your own:

```cpp
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"
#define FIREBASE_HOST "your-project-id.firebaseio.com"
#define FIREBASE_AUTH "your-firebase-database-secret"
#define BOT_TOKEN "your-telegram-bot-token"
#define CHAT_ID "your-chat-id"

```

### 6. ✅ Upload and Monitor

- Connect your ESP32 via USB.
- Select correct Board and Port in Arduino IDE.
- Upload the code.
- Open Serial Monitor to confirm readings and messages.

## 📁 Data Format in Firebase

Firebase path format:

```cpp
/Section1/Sensor1/2025-05-04_10:10:00/
    ├── Temperature: 28.5
    └── Humidity: 65.2
```

## 🛡️ Notes

- The current implementation uses `setInsecure()` for `WiFiClientSecure`. In production, use certificate validation.
- Ensure stable internet for real-time sync and messaging.
