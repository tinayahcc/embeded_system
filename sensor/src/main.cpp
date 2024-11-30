#include "DHT.h"

#define DHTPIN 33
#define DHTTYPE DHT11
#define TOUCH_PIN 27

#define RX (16)
#define TX (17)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RX, TX);
  dht.begin();

  // Initialize touch pin
  pinMode(TOUCH_PIN, INPUT);
}

void transmitDataToGatewayNode(String msg) {
  String header = "[FROM SENSOR NODE]";
  msg = header + msg;
  Serial1.println(msg);
}

void retrieveDataFromSensorNode() {
  if (Serial1.available()) {
    String data = Serial1.readStringUntil('\n');
    if (data.startsWith("[FROM GATEWAY NODE]")) {
      String content = data.substring(data.indexOf(']') + 1); // The actual content
      Serial.println("Received: " + content);
    }
  }
}

void loop() {
  float t = dht.readTemperature();  // Read temperature from DHT11 sensor
  int touchValue = digitalRead(TOUCH_PIN);  // Read touch sensor value
  
  // Determine touch status
  String touchStatus = (touchValue == 1) ? "Touched" : "Not Touched"; // Adjust threshold as needed
  
  // Combine data
  String data = "Temperature: " + String(t, 2) + "°C " + "Touch Status: " + touchStatus;

  // Transmit data to the gateway node
  transmitDataToGatewayNode(data);
  // retrieveDataFromSensorNode();

  delay(1000);  // Delay for 1 second
}
