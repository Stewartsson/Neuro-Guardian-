/* MODULE A: FALL DETECTOR */
#define BLYNK_TEMPLATE_ID "TMPLxxxx"      // PASTE YOUR TEMPLATE ID HERE
#define BLYNK_DEVICE_NAME "Neuro Guardian"
#define BLYNK_AUTH_TOKEN "YourToken_A"    // PASTE AUTH TOKEN FOR MODULE A

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

char ssid[] = "YourWiFiName";
char pass[] = "YourWiFiPassword";

Adafruit_MPU6050 mpu;
#define BUZZER_PIN 18
#define LED_PIN 19

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  Blynk.run();
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Calculate Total Acceleration Vector (Magnitude)
  float totalAccel = sqrt(pow(a.acceleration.x, 2) + 
                          pow(a.acceleration.y, 2) + 
                          pow(a.acceleration.z, 2));

  // Threshold: > 25 m/s^2 (approx 2.5G impact) indicates a hard fall
  if (totalAccel > 25.0) {
    Serial.println("FALL DETECTED!");
    
    // 1. Activate Local Alarm
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    
    // 2. Send Cloud Alert
    Blynk.logEvent("fall_alert", "EMERGENCY: Patient Fall Detected!");
    Blynk.virtualWrite(V3, "FALL DETECTED!");
    
    // 3. Keep Alarm on for 5 seconds
    delay(5000);
    
    // 4. Reset
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    Blynk.virtualWrite(V3, "Status: Safe");
  }
  delay(50);
}
