/* MODULE C: SMART HEAL THERANOSTICS */
#define BLYNK_TEMPLATE_ID "TMPLxxxx"      // PASTE YOUR TEMPLATE ID
#define BLYNK_DEVICE_NAME "Neuro Guardian"
#define BLYNK_AUTH_TOKEN "YourToken_C"    // PASTE AUTH TOKEN FOR MODULE C

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <math.h>

char ssid[] = "YourWiFiName";
char pass[] = "YourWiFiPassword";

// Pins for ESP32-C3 SuperMini
#define PIN_TEMP 0   // A0
#define PIN_PH   1   // A1
#define LED_G    3
#define LED_R    4
#define LED_B    5   // Therapy Blue Light

// Steinhart-Hart Coefficients for NTC Thermistor
const double VCC = 3.3; 
const double R_DIV = 10000.0;
const double A = 0.001129148; 
const double B = 0.000234125; 
const double C = 0.0000000876741;

BlynkTimer timer;
bool manualOverride = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_G, OUTPUT); 
  pinMode(LED_R, OUTPUT); 
  pinMode(LED_B, OUTPUT);
  pinMode(PIN_TEMP, INPUT); 
  pinMode(PIN_PH, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, runLogic);
}

// Doctor Button on V5
BLYNK_WRITE(V5) {
  if (param.asInt() == 1) {
    manualOverride = true;
  } else {
    manualOverride = false;
  }
}

void runLogic() {
  // 1. Read Temp
  int rawTemp = analogRead(PIN_TEMP);
  double Vout = (rawTemp * VCC) / 4095.0;
  double R_NTC = (VCC * R_DIV / Vout) - R_DIV;
  double logR = log(R_NTC);
  double tempC = (1.0 / (A + B * logR + C * logR * logR * logR)) - 273.15;

  // 2. Read pH (Simulated)
  int rawPH = analogRead(PIN_PH);
  double phVal = (rawPH * 3.3 / 4095.0) * 1.21 + 5.0; // Simulated 5-9 range

  // 3. Logic: Infection Detected OR Doctor Override
  bool infection = (tempC > 37.5 || phVal > 8.0);
  
  if (infection || manualOverride) {
    // CRITICAL MODE
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH); // THERAPY ON
    Blynk.virtualWrite(V2, "CRITICAL: Therapy Active");
  } else {
    // SAFE MODE
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
    Blynk.virtualWrite(V2, "Healing Normal");
  }

  // Send Data to Blynk
  Blynk.virtualWrite(V0, tempC);
  Blynk.virtualWrite(V1, phVal);
}

void loop() {
  Blynk.run();
  timer.run();
}

