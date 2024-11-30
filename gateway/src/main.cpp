#include <Arduino.h>
#include <Firebase.h>
#include <WiFi.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Define pins and sensors
#define RX (16)
#define TX (17)

#define WIFI_SSID "wrwwmt"
#define WIFI_PASSWORD "1594873H"
#define FIREBASE_API_KEY "AIzaSyAKPSmr1CdJpGhxSKOqkdeeQqbJJkvq1JY"
#define DB_URL "https://embeddedproject-16af6-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void retrieveDataFromSensorNode() {
  if (Serial1.available()) {
    String data = Serial1.readStringUntil('\n');
    if (data.startsWith("[FROM SENSOR NODE]")) {
      Serial.println(data);
      String content = data.substring(data.indexOf(']')+1); // The actual content
    }
  }
}

void transmitDataToSensorNode(String msg) {
  String header = "[FROM GATEWAY NODE]";
  Serial.println("Sending Data to Sensor Node with content: " + msg);
  msg = header + msg;
  Serial1.println(msg);
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RX, TX);

  // wifi connection
  // Serial.print("Connecting to wifi..");
  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print(".");
  //   delay(300);
  // }
  // Serial.println("\nWifi Connected successfully.");

  // Firebase
  // config.api_key = FIREBASE_API_KEY;
  // config.database_url = DB_URL;
  // config.timeout.serverResponse = 10000;
  // if (Firebase.signUp(&config, &auth, "", "")) {
  //   Serial.println("Sign up to Firebase successfully");
  // } else {
  //   Serial.println("Unable to sign up to Firebase");
  //   Serial.println(WiFi.status());
  //   Serial.println("Error message: " + String(config.signer.signupError.message.c_str()));
  // }

  // config.token_status_callback = tokenStatusCallback;
  // Firebase.begin(&config, &auth);
  // Firebase.reconnectWiFi(true);
}

void loop() {
  retrieveDataFromSensorNode();
}