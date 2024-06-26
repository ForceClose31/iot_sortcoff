#include <ESP32Servo.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "Connecting..."
#define WIFI_PASSWORD "1234abcd"

#define DATABASE_URL "https://authshortcoff-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyDtcJAoeqBXmxlsZsp048-XnKMDO8XQ70E"
#define FIREBASE_PROJECT_ID "authshortcoff"

FirebaseData fbdo;
FirebaseAuth firebaseAuth;
FirebaseConfig firebaseConfig;

FirebaseJson json;

#define S0 17
#define S1 16
#define S2 27
#define S3 14
#define sensorOut 23

Servo topServo;
Servo secondServo;

int redFrequency = 0;
int yellowFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

int redColor = 0;
int yellowColor = 0;
int greenColor = 0;
int blueColor = 0;

int redCount = 0;
int yellowCount = 0;
int blueCount = 0;
int greenCount = 0;

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  firebaseConfig.api_key = API_KEY;
  firebaseConfig.database_url = DATABASE_URL;

  if (Firebase.signUp(&firebaseConfig, &firebaseAuth, "", "")) {
    Serial.println("Signed up");
  } else {
    Serial.printf("Failed to sign up: %s\n", fbdo.errorReason().c_str());
  }

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);

  ESP32PWM::allocateTimer(0);
  topServo.setPeriodHertz(50);
  topServo.attach(12, 500, 2500);

  secondServo.setPeriodHertz(50);
  secondServo.attach(13, 500, 2500);
}

void detectYellowSensor() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  yellowFrequency = pulseIn(sensorOut, LOW, 500);
  yellowColor = map(yellowFrequency, 115, 171, 255, 0);

  Serial.print(" Y = ");
  Serial.print(yellowColor);
  delay(500);
}

void detectGreenSensor() {
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  greenFrequency = pulseIn(sensorOut, LOW, 500);
  greenColor = map(greenFrequency, 25, 54, 255, 0);

  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  redFrequency = pulseIn(sensorOut, LOW, 500);
  redColor = map(redFrequency, 25, 55, 255, 0);

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  blueFrequency = pulseIn(sensorOut, LOW, 500);
  blueColor = map(blueFrequency, 24, 55, 255, 0);

  Serial.print(" B = ");
  Serial.print(blueColor);

  Serial.print(" R = ");
  Serial.print(redColor);

  Serial.print(" G = ");
  Serial.print(greenColor);
  delay(500);
}

void detectColorAndMoveServo() {
  detectGreenSensor();
  if ((redColor - greenColor) > 15 && redColor > 5 && redColor < 150) {
    topServo.write(20);
    Serial.println(" - RED detected!");
    redCount += 1;
  } else if ((redColor - greenColor) < 15 && redColor > 5 && redColor <= 150) {
    topServo.write(90);
    Serial.println(" - GREEN detected!");
    greenCount += 1;
  } else {
    if (yellowColor >= 150 && yellowColor <= 190) {
      Serial.println(" - YELLOW detected!");
      yellowCount += 1;
      topServo.write(50);
    } else {
      blueCount += 1;
      topServo.write(50);
      Firebase.RTDB.setInt(&fbdo, "/test/blue", blueCount);
    }
  }

  delay(500);
  secondServo.write(90);
  delay(500);

  Firebase.RTDB.setInt(&fbdo, "/test/green", greenCount);
  Firebase.RTDB.setInt(&fbdo, "/test/yellow", yellowCount);
  Firebase.RTDB.setInt(&fbdo, "/test/red", redCount);

  Serial.print("blue : ");
  Serial.println(blueCount);
  Serial.print("green : ");
  Serial.println(greenCount);
  Serial.print("red : ");
  Serial.println(redCount);
  Serial.print("yellow : ");
  Serial.println(yellowCount);
  delay(500);
}

void loop() {
  secondServo.write(0);

  if (Firebase.RTDB.getInt(&fbdo, "/test/on")) {
    if (fbdo.dataType() == "int" && fbdo.intData() == 1) {
      String currentUID = fbdo.stringData();
      detectColorAndMoveServo();
    }
  } else {
    Serial.println("Failed to get value from Firebase");
  }
  delay(1000);
}
 