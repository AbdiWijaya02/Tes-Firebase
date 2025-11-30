#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <PZEM004Tv30.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <FirebaseESP8266.h>

#define PZEM_TX 14  // D5 (GPIO14)
#define PZEM_RX 12  // D6 (GPIO12)
#define DHTPIN 2    // D4 (GPIO2)
#define DHTTYPE DHT11
#define SSR_PIN 0   // D3 (GPIO0)
#define CNT_PIN 16 // D0 (GPIO16)
#define FINGER_TX 13 // D7 (GPIO13)
#define FINGER_RX 15 // D8 (GPIO15)
#define LCD_SDA 5    // D1 (GPIO5)
#define LCD_SCL 4    // D2 (GPIO4)

// WiFi credentials
const char* ssid = "Pro";
const char* password = "12345678";

// Firebase credentials
#define FIREBASE_HOST "https://iot-simalas-default-rtdb.asia-southeast1.firebasedatabase.app/"  // Ganti dengan Firebase Host Anda
#define FIREBASE_AUTH "1g47DDP3FbTq4vN9h6Zr26682xCDvVT77LsZbU4P"  // Ganti dengan Database Secret Anda

PZEM004Tv30 pzem(PZEM_RX, PZEM_TX); // PZEM RX, TX
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float Dia = 0.03286;  // Diameter roda counter (dalam meter)
int CPR = 10;  // Jumlah pickup pada roda counter
float FullRoll = 330.0;  // Panjang standar satu spool filament (dalam meter)
float Remaining = 330.0;  // Panjang filament yang tersisa
float Used = 0.0, OldUsed = 0.0;
float LowLevelLimit = 5.0;  // Batas panjang filament rendah

long newPosition = 0;
long oldPosition = -999;
long Click = 0;
unsigned long LowTime = 0;

bool ssrState = false; // Status SSR, false = OFF, true = ON
bool Saved = false;
bool Started = false;
bool Low = false;
bool LowLevel = false;
bool LevelLimit = false;

// Firebase objects
FirebaseData fbData;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  pinMode(CNT_PIN, INPUT_PULLUP);
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);
  dht.begin();
  lcd.init();
  lcd.backlight();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Setup Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Firebase Ready");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Membaca data dari sensor
    float tegangan = pzem.voltage();
    float arus = pzem.current();
    float daya = pzem.power();
    float energi = pzem.energy();
    float suhu = dht.readTemperature();
    float kelembapan = dht.readHumidity();

    if ((Used - OldUsed) > 1.0) {
      OldUsed = Used;
      EEPROM.put(0, Remaining);
    }
    // Jika sinyal rendah, mulai pengukuran waktu
    if (digitalRead(CNT_PIN) == LOW) {
      LowTime = millis();
      Low = true;
    }
    // Menghitung panjang filament yang digunakan
    if (Low && (digitalRead(CNT_PIN) == HIGH)) {
      if (millis() - LowTime > 1000) {
        Started = true;
        Low = false;
        LowTime = millis();
        Click++;
        Used += (Dia * 3.1415927) / CPR;  // Perhitungan dalam meter
        Remaining -= (Dia * 3.1415927) / CPR;  // Mengurangi panjang filament yang tersisa
      }
    }

    // Mengirim data ke Firebase
    Firebase.setFloat(fbData, "sensor/tegangan", tegangan);
    Firebase.setFloat(fbData, "sensor/arus", arus);
    Firebase.setFloat(fbData, "sensor/daya", daya);
    Firebase.setFloat(fbData, "sensor/energi", energi);
    Firebase.setFloat(fbData, "sensor/suhu", suhu);
    Firebase.setFloat(fbData, "sensor/kelembapan", kelembapan);
    Firebase.setFloat(fbData, "sensor/used", Used);       // Panjang filament yang digunakan
    Firebase.setFloat(fbData, "sensor/remaining", Remaining); // Panjang filament yang tersisa
    Serial.println("Value sent to Firebase: " + String(tegangan));
    Serial.println("Value sent to Firebase: " + String(arus));
    Serial.println("Value sent to Firebase: " + String(daya));
    Serial.println("Value sent to Firebase: " + String(energi));
    Serial.println("Value sent to Firebase: " + String(suhu));
    Serial.println("Value sent to Firebase: " + String(kelembapan));
    Serial.println("Value sent to Firebase: " + String(Used));
    Serial.println("Value sent to Firebase: " + String(Remaining));

    // Baca status SSR dari Firebase
    if (Firebase.getBool(fbData, "control/ssrState")) {
      bool newSsrState = fbData.boolData();
      if (newSsrState != ssrState) {
        ssrState = newSsrState;
        digitalWrite(SSR_PIN, ssrState ? HIGH : LOW);
        Serial.println(ssrState ? "SSR ON" : "SSR OFF");
      }
    }

    delay(1000);  // Tunggu 1 detik sebelum loop berikutnya
  }
}
