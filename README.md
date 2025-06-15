# MailBot: ESP32 ve Arduino Tabanlı Akıllı Posta Kutusu

MailBot, gelen bir e-posta bildirimiyle fiziksel tepki veren bir IoT projesidir. ESP32 üzerinden Telegram Bot kullanılarak gelen mesajlar algılanmakta ve buna bağlı olarak LED, buzzer, OLED ekran ve servo motor ile kullanıcıya bildirim sağlanmaktadır. Servo motor, ESP32'nin zarar görmesini önlemek amacıyla ayrı bir Arduino Uno ile kontrol edilmektedir.

## Proje Amacı

Bu projenin amacı, kullanıcıların gelen e-posta bildirimlerini fiziksel çıktılar yoluyla fark etmelerini sağlamaktır. Sistem, özellikle evden çalışan bireyler veya e-posta ile önemli bilgi alan öğrenciler için işlevsel bir çözümdür.

## Kullanılan Donanım Bileşenleri

- **ESP32** – WiFi bağlantısı ve Telegram mesaj kontrolü için
- **Arduino Uno** – Servo motorun güvenli kontrolü için
- **Servo Motor** – Kartondan yapılmış posta kutusunun kapağını açmak için
- **LED** – Görsel uyarı sağlamak için
- **Buzzer** – Sesli uyarı vermek için
- **OLED Ekran** – Yazılı bildirim göstermek için
- **Breadboard & Jumper Kabloları** – Devre bağlantıları için

## Kurulum ve Yapılandırma

1. Arduino IDE kurulumu gerçekleştirilmelidir.
2. Gerekli kütüphaneler:
   - WiFi.h
   - HTTPClient.h
   - ArduinoJson
   - Adafruit_SSD1306
   - ESP32Servo

3. Telegram bot kurulumu:
   - `@BotFather` üzerinden bot oluşturun.
   - Token alın ve kodda ilgili bölüme yapıştırın.

4. Donanım bağlantıları tamamlanmalıdır:
   - ESP32’ye LED, buzzer, OLED ekran bağlanır.
   - Servo motor, Arduino üzerinden kontrol edilir.
   - ESP32 ve Arduino dijital pin ile haberleştirilir.

## Çalışma Prensibi

- Kullanıcı Telegram üzerinden “/mailgeldi” komutunu gönderir.
- ESP32 bu komutu algılar ve:
  - Buzzer ses verir,
  - OLED ekranda mesaj görüntülenir.
- Arduino, ESP32’den aldığı tetikleme ile servo motoru hareket ettirir.

## Neden Arduino + ESP32?

Servo motorun ESP32 üzerinden çalıştırılması, yüksek akım çekişi nedeniyle ESP32’ye zarar verebilir. Bu nedenle, servo motor bağımsız bir Arduino Uno üzerinden kontrol edilmiştir.

## Lisans

Bu proje, Ankara Üniversitesi Bilgisayar ve Öğretim Teknolojileri Eğitimi bölümü kapsamında, BOZ214 dersi için geliştirilmiştir. Kişisel ve akademik kullanım için açıktır.

## Geliştirici

**Meziyet Yaren Çevik**  
Ankara Üniversitesi, Bilgisayar ve Öğretim Teknolojileri Öğretmenliği
Email: meziyetyarencevik@gmail.com
