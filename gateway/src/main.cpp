#include <Arduino.h>
#include <Firebase.h>
#include <WiFi.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Define pins and sensors
#define RX (16)
#define TX (17)
#define LDR_PIN 34
#define IR_PIN 2

// Define ir, led, ldr

#define WIFI_SSID "pcyn"
#define WIFI_PASSWORD "punch514114"
#define FIREBASE_API_KEY "AIzaSyAKPSmr1CdJpGhxSKOqkdeeQqbJJkvq1JY"
#define DB_URL "https://embeddedproject-16af6-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPreMillis = 0;
bool signupOK = false;
float ldrData = 0;
float voltage = 0.0;


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
  Serial.print("Connecting to wifi..");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWifi Connected successfully.");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Firebase
  config.api_key = FIREBASE_API_KEY;
  config.database_url = DB_URL;
  config.timeout.serverResponse = 10000;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign up to Firebase successfully");
    signupOK = true;
  } else {
    Serial.println("Unable to sign up to Firebase");
    Serial.println(WiFi.status());
    Serial.println("Error message: " + String(config.signer.signupError.message.c_str()));
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  //retrieveDataFromSensorNode();

  if(Firebase.ready() && signupOK && (millis() - sendDataPreMillis > 5000 || sendDataPreMillis == 0)){
    sendDataPreMillis = millis();
    retrieveDataFromSensorNode();
    ldrData = analogRead(LDR_PIN);
    voltage = (float)analogReadMilliVolts(LDR_PIN)/1000;

    // sent data to database
    // ldr
    if(Firebase.RTDB.setFloat(&fbdo, "/Sensor/LDR", ldrData)){
      Serial.println();
      Serial.print(ldrData);
      Serial.println("(" + fbdo.dataType() + ")");
    }else{
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    // voltage
    // if(Firebase.RTDB.setFloat(&fbdo, "voltage", voltage)){
    //   Serial.println();
    //   Serial.print(voltage);
    //   Serial.println("(" + fbdo.dataType() + ")");
    // }else{
    //   Serial.println("FAILED: " + fbdo.errorReason());
    // }

    // temperature
    // if(Firebase.RTDB.setFloat(&fbdo, "temperature", temperature)){
    //   Serial.println();
    //   Serial.print(temperature);
    //   Serial.println("(" + fbdo.dataType() + ")");
    // }else{
    //   Serial.println("FAILED: " + fbdo.errorReason());
    // }

    // touch
    // if(Firebase.RTDB.setBool(&fbdo, "touched", touched)){
    //   Serial.println();
    //   Serial.print(touched);
    //   Serial.println("(" + fbdo.dataType() + ")");
    // }else{
    //   Serial.println("FAILED: " + fbdo.errorReason());
    // }

  }
}