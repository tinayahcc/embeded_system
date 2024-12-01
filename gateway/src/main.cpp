#include <Arduino.h>
#include <Firebase.h>
#include <WiFi.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <LiquidCrystal_I2C.h>

// Define pins and sensors
#define RX (16)
#define TX (17)
#define LDR_PIN 34
#define IR_PIN 13
#define RED_PIN 5
#define GREEN_PIN 18
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define FIREBASE_API_KEY "AIzaSyAKPSmr1CdJpGhxSKOqkdeeQqbJJkvq1JY"
#define DB_URL "https://embeddedproject-16af6-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPreMillis = 0;
bool signupOK = false;

float prev_ldrData = 0.0;
float prev_temperature = 0.0;
bool prev_touchStatus = false;
int prev_IrData = 1;

float ldrData = 0;
float temperature = 0.0;
bool touchStatus = false;

String tempString;
String touchString;
int IrData;
String IDValue;

void retrieveDataFromSensorNode() {
  if (Serial1.available()) {
    String data = Serial1.readStringUntil('\n');
    if (data.startsWith("[FROM SENSOR NODE]")) {
      //Serial.println(data);
      //String content = data.substring(data.indexOf(']')+1); // The actual content

      // temperature sensor
      int tempStart = data.indexOf("Temperature: ") + 13;
      int tempEnd = data.indexOf(" °C");
      tempString = data.substring(tempStart, tempEnd);
      temperature = tempString.toFloat();

      // touch sensor
      int touchStart = data.indexOf("Touch Status: ") + 14;
      touchString = data.substring(touchStart);
      touchString.trim(); // Remove unwanted spaces or newline characters

      if (touchString.equals("Touched")) {
        touchStatus = true;
      } else if (touchString.equals("Not Touched")) {
        touchStatus = false;
      }

      // Serial.println(temperature);
      // Serial.print("Touch Status: ");
      // Serial.println(touchString);
      // Serial.println(touchStatus);
    }
  }
}

// void transmitDataToSensorNode(String msg) {
//   String header = "[FROM GATEWAY NODE]";
//   Serial.println("Sending Data to Sensor Node with content: " + msg);
//   msg = header + msg;
//   Serial1.println(msg);
// }

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RX, TX);

  lcd.init();
  lcd.backlight(); // Turn on the backlight

  lcd.setCursor(0, 0); // Set cursor to column 0, row 0
  lcd.setCursor(0, 1); // Set cursor to column 0, row 1

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  // Initialize LED to off
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);

  int wifi_optiion = 1;
  String Wifi_SSIDs[2] = {"wrwwmt", "pcyn"};
  String Wifi_Pass[2] = {"1594873H", "punch514114"};

  String WIFI_SSID = Wifi_SSIDs[wifi_optiion];
  String WIFI_PASSWORD = Wifi_Pass[wifi_optiion];
  // String WIFI_SSID = "hrnph";
  // String WIFI_PASSWORD = "00000000";

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

  //Firebase
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
  retrieveDataFromSensorNode();

  if(Firebase.ready() && signupOK && (millis() - sendDataPreMillis > 5000 || sendDataPreMillis == 0)){
    sendDataPreMillis = millis();
    //retrieveDataFromSensorNode();
    ldrData = analogRead(LDR_PIN);
    IrData = digitalRead(IR_PIN);

    // IR
    Serial.println("======= Firebase sensors update status =======");
    if(prev_IrData != IrData && Firebase.RTDB.setInt(&fbdo, "/Sensor/IR", IrData)){
      prev_IrData= IrData;
      Serial.println("Update IR sensor to: " + String(IrData) + " ( " + fbdo.dataType() + " ) ");
    }

    // ldr
    Serial.println("======= Firebase sensors update status =======");
    if(abs(prev_ldrData - ldrData) > 300 && Firebase.RTDB.setFloat(&fbdo, "/Sensor/LDR", ldrData)){
      prev_ldrData = ldrData;
      Serial.println("Update LDR sensor to: " + String(ldrData) + " ( " + fbdo.dataType() + " ) ");
    }
    // temperature
    if(abs(prev_temperature - temperature) > 2 &&  Firebase.RTDB.setFloat(&fbdo, "/Sensor/Temperature", temperature)){
      prev_temperature = temperature;
      Serial.println("Update Temperature sensor to: " + String(temperature) + " ( " + fbdo.dataType() + " ) ");
    }
    // touch
    if(prev_touchStatus != touchStatus && Firebase.RTDB.setBool(&fbdo, "/Sensor/Touched", touchStatus)){
      prev_touchStatus = touchStatus;
      Serial.println("Update Touch sensor to: " + String(touchStatus) + " ( " + fbdo.dataType() + " ) ");
    }

    Serial.println("============= Lastest Attendance =============");
    // read data from firebase to esp32
    if(Firebase.RTDB.getJSON(&fbdo, "/attendance")){
      FirebaseJson json = fbdo.to<FirebaseJson>();
      FirebaseJsonData jsonData;
      String key, value;
      String student_id;
      bool is_late;
      int type;
      Serial.println("Debug: " + String((int)json.iteratorBegin()));
      json.iteratorGet((int)json.iteratorBegin() - 2, type, key, value);
      student_id = value;
      json.iteratorGet((int)json.iteratorBegin() - 3, type, key, value);
      is_late = (value == "false") ? false : true;
      // write to LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(student_id);

      Serial.println("Student ID: " + student_id);
      Serial.println("Late: " + value);

      // Serial.print("Test studnet id: ");
      // Serial.println(student_id);

      // change RGB led status
      if(is_late){
        //digitalWrite(RED_PIN, HIGH);  // Turn on red
        digitalWrite(GREEN_PIN, LOW); // Turn off green
      }else if(!is_late){
        digitalWrite(RED_PIN, LOW);   // Turn off red
        //digitalWrite(GREEN_PIN, HIGH);// Turn on green
      }

    } else {
      Serial.println("HOIYAH");
    }
    Serial.println("==============================================\n");

  }
}