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
int redFrequency2 = 0;
int greenFrequency = 0;
int greenFrequency2 = 0;
int blueFrequency = 0;

int redColor = 0;
int redColor2 = 0;
int redColor3 = 0;
int greenColor = 0;
int greenColor2 = 0;
int blueColor = 0;

int redCount = 0;
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

void readColorSensor() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  redFrequency = pulseIn(sensorOut, LOW, 500);
  redColor = map(redFrequency, 25, 55, 255, 0);
  // redColor2 = map(redFrequency, 104, 198, 255, 0);
  // redColor3 = map(redFrequency, 139, 151, 255, 0);

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  greenFrequency = pulseIn(sensorOut, LOW, 500);
  greenColor = map(greenFrequency, 25, 54, 255, 0);
  // greenColor2 = map(greenFrequency, 118, 182, 255, 0);

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  blueFrequency = pulseIn(sensorOut, LOW, 500);
  blueColor = map(blueFrequency, 24, 55, 255, 0);

  Serial.print("R = ");
  Serial.print(redColor);
  // Serial.print(" R 2 = ");
  // Serial.print(redColor2);
  // Serial.print(" R 3 = ");
  // Serial.print(redColor3);
  Serial.print(" G = ");
  Serial.print(greenColor);
  // Serial.print(" G 2 = ");
  // Serial.print(greenColor2);
  Serial.print(" B = ");
  Serial.print(blueColor);
  delay(2500);
}

void detectColorAndMoveServo() {
  readColorSensor();

  if (redColor > 115 && redColor < 250 && greenColor > 100 && greenColor < 250 && blueColor > 140 && blueColor < 260) {
    Serial.println(" - GREEN detected!");
    greenCount += 1;
    topServo.write(20);
  } else if (blueColor > redColor && blueColor > greenColor) {
    Serial.println(" - BLUE detected!");
    blueCount += 1;
    topServo.write(50);

    Firebase.RTDB.setInt(&fbdo, "/test/blue", blueCount);
  } else if (redColor > 50 && redColor < 80 && greenColor > 0 && greenColor < 50 && blueColor > 20 && blueColor < 50) {
    Serial.println(" - RED detected!");
    redCount += 1;
    topServo.write(90);
  }

  delay(500);             
  secondServo.write(90);  
  delay(500);             

  Firebase.RTDB.setInt(&fbdo, "/test/green", greenCount);
  Firebase.RTDB.setInt(&fbdo, "/test/red", redCount);

  Serial.print("blue : ");
  Serial.println(blueCount);
  Serial.print("green : ");
  Serial.println(greenCount);
  Serial.print("red : ");
  Serial.println(redCount);
  delay(500);
}

void loop() {
  secondServo.write(0);

  if (Firebase.RTDB.getInt(&fbdo, "/test/blue")) {
    int currentBlueCount = fbdo.intData();

    if (currentBlueCount >= 100) {
      Firebase.RTDB.setInt(&fbdo, "/test/on", 0);
      Serial.println("Blue count reached 100. Stopping program.");
      while (true) {
      }
    }
  } else {
    Serial.println("Failed to get blue count from Firebase");
  }

  if (Firebase.RTDB.getInt(&fbdo, "/test/on")) {
    if (fbdo.dataType() == "int" && fbdo.intData() == 1) {
      String currentUID = fbdo.stringData();            
      detectColorAndMoveServo();
    }
  } else {
    Serial.println("Failed to get value from Firebase");
  }
  delay(1000);  // Check the database every second
}