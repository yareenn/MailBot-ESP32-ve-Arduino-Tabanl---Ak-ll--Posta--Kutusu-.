#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ═══════════════════════════════════════════════════════════════
// 🔧 AYARLAR - SADECE BOT TOKEN'I DEĞİŞTİRİN!
// ═══════════════════════════════════════════════════════════════

// WiFi Bilgileri
const char* ssid = "YarenHotSpot";
const char* password = "yarenn1234";

// Telegram Bot Bilgileri
String botToken = "7559759253:AAEcAzJiVDSc_cUYgJH7tlIVOwEXOBOgMos";  // 👈 BURAYA BOT TOKEN'INIZI YAZIN!
String chatID = "1156410367";             // ✅ Chat ID hazır

// ═══════════════════════════════════════════════════════════════
// 🔌 DONANIM AYARLARI
// ═══════════════════════════════════════════════════════════════

// Pin Tanımlamaları (Donanım bağlantılarınıza göre)
#define BUZZER_PIN 26      // Buzzer → ESP32 GPIO 26
#define SERVO_PIN 27       // Servo Sinyal → ESP32 GPIO 27
#define OLED_SDA 21        // OLED SDA → ESP32 GPIO 21
#define OLED_SCL 22        // OLED SCL → ESP32 GPIO 22

// OLED Ekran Ayarları
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Servo Motor
Servo mailboxServo;

// ═══════════════════════════════════════════════════════════════
// 📊 SISTEM DEĞİŞKENLERİ
// ═══════════════════════════════════════════════════════════════

unsigned long lastCheck = 0;
const unsigned long checkInterval = 3000; // 3 saniyede bir kontrol
int lastUpdateID = 0;
bool mailboxOpen = false;
bool systemReady = false;

void setup() {
  Serial.begin(115200);
  Serial.println("🤖 MailBot v2.0 Başlatılıyor...");
  
  // ═══════════════════════════════════════════════════════════════
  // 🔧 DONANIM BAŞLATMA
  // ═══════════════════════════════════════════════════════════════
  
  // Pin ayarları
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Servo motor başlatma
  mailboxServo.attach(SERVO_PIN);
  mailboxServo.write(0);  // Başlangıçta kapalı pozisyon
  Serial.println("✅ Servo motor hazır (0 derece)");
  
  // OLED ekran başlatma
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ OLED başlatılamadı! Bağlantıları kontrol edin!");
    // OLED yoksa da devam et
  } else {
    Serial.println("✅ OLED ekran hazır");
  }
  
  // Başlangıç ekranı
  showDisplay("MailBot v2.0", "Sistem baslatiliyor...", "WiFi baglaniyor...", "");
  
  // Başlangıç sesi
  playStartupSound();
  
  // ═══════════════════════════════════════════════════════════════
  // 🌐 WiFi BAĞLANTISI
  // ═══════════════════════════════════════════════════════════════
  
  WiFi.begin(ssid, password);
  int wifiAttempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ WiFi bağlantısı başarılı!");
    Serial.print("📡 IP Adresi: ");
    Serial.println(WiFi.localIP());
    
    // Başarılı bağlantı ekranı
    showDisplay("MailBot v2.0", "WiFi: BAGLI", "IP: " + WiFi.localIP().toString(), "Mail bekleniyor...");
    
    // Başarı sesi
    playSuccessSound();
    systemReady = true;
    
    // Telegram'a sistem başlatıldı mesajı gönder
    sendTelegramMessage("🤖 MailBot sistemi başlatıldı ve hazır! Mesaj gönderebilirsiniz.");
    
  } else {
    Serial.println("❌ WiFi bağlantısı başarısız!");
    showDisplay("MailBot v2.0", "WiFi: HATA!", "Baglanti basarisiz", "Yeniden deneyin");
    playErrorSound();
  }
  
  Serial.println("🚀 MailBot hazır! Telegram'dan mesaj gönderebilirsiniz.");
}

void loop() {
  // Sistem hazırsa Telegram mesajlarını kontrol et
  if (systemReady && WiFi.status() == WL_CONNECTED) {
    if (millis() - lastCheck > checkInterval) {
      checkTelegramMessages();
      lastCheck = millis();
    }
  } else if (WiFi.status() != WL_CONNECTED) {
    // WiFi bağlantısı kesildi
    Serial.println("❌ WiFi bağlantısı kesildi!");
    showDisplay("MailBot v2.0", "WiFi: KESILDI!", "Yeniden baglan", "bekleyin...");
    systemReady = false;
    
    // WiFi'yi yeniden bağla
    WiFi.reconnect();
    delay(5000);
  }
  
  delay(100);
}

// ═══════════════════════════════════════════════════════════════
// 📨 TELEGRAM MESAJ KONTROL FONKSİYONU
// ═══════════════════════════════════════════════════════════════

void checkTelegramMessages() {
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + botToken + "/getUpdates?offset=" + String(lastUpdateID + 1);
  
  http.begin(url);
  http.setTimeout(5000); // 5 saniye timeout
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    // JSON parse et
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      JsonArray results = doc["result"];
      
      for (JsonObject result : results) {
        int updateID = result["update_id"];
        lastUpdateID = updateID;
        
        if (result.containsKey("message")) {
          JsonObject message = result["message"];
          String messageText = message["text"];
          String senderName = message["from"]["first_name"];
          
          Serial.println("📨 Yeni mesaj geldi!");
          Serial.println("👤 Gönderen: " + senderName);
          Serial.println("💬 Mesaj: " + messageText);
          
          // Yeni mail geldi - sistemi aktive et!
          handleNewMail(senderName, messageText);
        }
      }
    } else {
      Serial.println("❌ JSON parse hatası: " + String(error.c_str()));
    }
  } else {
    Serial.println("❌ HTTP isteği başarısız: " + String(httpCode));
  }
  
  http.end();
}

// ═══════════════════════════════════════════════════════════════
// 📬 YENİ MAİL GELDİ FONKSİYONU
// ═══════════════════════════════════════════════════════════════

void handleNewMail(String sender, String message) {
  Serial.println("🎉 YENİ MAİL SİSTEMİ AKTİVE EDİLDİ!");
  
  // 1. OLED ekranda mesajı göster
  showDisplay("YENI MAIL GELDI!", "Gonderen: " + sender, message.substring(0, 20), "Kutu aciliyor...");
  
  // 2. Mail geldi sesi çal
  playMailAlert();
  
  // 3. Posta kutusunu aç (servo motor)
  openMailbox();
  
  // 4. Telegram'a geri bildirim gönder
  sendTelegramMessage("✅ Mail bildirimi alındı! MailBot sistemi aktive edildi! 📫✨");
  
  // 5. 8 saniye bekle (kullanıcı postayı görsün)
  delay(8000);
  
  // 6. Sistemi sıfırla
  resetSystem();
}

// ═══════════════════════════════════════════════════════════════
// 🔧 SERVO MOTOR FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

void openMailbox() {
  if (!mailboxOpen) {
    Serial.println("📬 Posta kutusu açılıyor...");
    
    // Servo'yu direkt 90 dereceye götür
    mailboxServo.write(90); // 90 dereceye hareket et
    delay(500); // Yavaşça hareket etmesi için kısa bir bekleme ekleyin (500 ms)
    
    mailboxOpen = true;
    Serial.println("✅ Posta kutusu açıldı (90 derece)");
  }
}

void closeMailbox() {
  if (mailboxOpen) {
    Serial.println("📪 Posta kutusu kapanıyor...");
    
    // Servo'yu direkt 0 dereceye götür
    mailboxServo.write(0); // 0 dereceye hareket et
    delay(500); // Yavaşça hareket etmesi için kısa bir bekleme ekleyin (500 ms)
    
    mailboxOpen = false;
    Serial.println("✅ Posta kutusu kapandı (0 derece)");
  }
}

// ═══════════════════════════════════════════════════════════════
// 🖥️ OLED EKRAN FONKSİYONU
// ═══════════════════════════════════════════════════════════════

void showDisplay(String line1, String line2, String line3, String line4) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 16);
  display.println(line2);
  display.setCursor(0, 32);
  display.println(line3);
  display.setCursor(0, 48);
  display.println(line4);
  
  display.display();
}

// ═══════════════════════════════════════════════════════════════
// 🔊 SES FONKSİYONLARI (AKTİF BUZZER İÇİN)
// ═══════════════════════════════════════════════════════════════

void playMailAlert() {
  Serial.println("🔔 Mail uyarı sesi çalıyor...");
  
  // Mail geldi melodisi (aktif buzzer için)
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
  
  delay(300);
  
  // Uzun bip
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
}

void playStartupSound() {
  Serial.println("🎵 Başlangıç sesi...");
  
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
}

void playSuccessSound() {
  Serial.println("✅ Başarı sesi...");
  
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

void playErrorSound() {
  Serial.println("❌ Hata sesi...");
  
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
}

// ═══════════════════════════════════════════════════════════════
// 🔄 SİSTEM SIFIRLAMA FONKSİYONU
// ═══════════════════════════════════════════════════════════════

void resetSystem() {
  Serial.println("🔄 Sistem sıfırlanıyor...");
  
  // Posta kutusunu kapat
  closeMailbox();
  
  // Varsayılan ekrana dön
  showDisplay("MailBot v2.0", "WiFi: BAGLI", "IP: " + WiFi.localIP().toString(), "Mail bekleniyor...");
  
  Serial.println("✅ Sistem sıfırlandı, yeni mail bekleniyor...");
}

// ═══════════════════════════════════════════════════════════════
// 📤 TELEGRAM MESAJ GÖNDERME FONKSİYONU
// ═══════════════════════════════════════════════════════════════

void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + botToken + "/sendMessage";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    String jsonString = "{\"chat_id\":\"" + chatID + "\",\"text\":\"" + message + "\"}";
    
    int httpCode = http.POST(jsonString);
    
    if (httpCode == 200) {
      Serial.println("📤 Telegram mesajı gönderildi: " + message);
    } else {
      Serial.println("❌ Telegram mesajı gönderilemedi: " + String(httpCode));
    }
    
    http.end();
  }
}
