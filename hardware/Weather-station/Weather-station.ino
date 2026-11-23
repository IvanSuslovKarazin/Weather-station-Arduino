#include <DHT.h> // підключення бібліотеки для DHT
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // підключення бібліотеки для ЖК

#define DHT_PIN 2
#define DHT_TYPE DHT11
#define PHOTORESISTOR_PIN A0
#define LED_RED_PIN 4
#define BUTTON_PIN 7

boolean ledState = false; // false — вимкнений, true — увімкнений для червоного світлодіоду;
boolean dhtEnabled = false; // Потік через DHT11;
boolean forceRead = false; // Прапорець для миттєвого першого заміру після натискання кнопки
boolean lastButtonState = HIGH; // останній стан кнопки;
unsigned long lastReadTime = 0; // реалізування через millis();
unsigned long readInterval = 15000; // Прийом інтервалу з сервера(15 за замовчуванням)
unsigned long lastLightTime = 0;          // Таймер для швидкої перевірки освітлення
const unsigned long LIGHT_INTERVAL = 250; // Перевірка світла кожні 250 мс (0.25 сек)
int lightPercent = 0; 
boolean autoBacklight = true;       // true — авторежим підсвітки по фоторезистору, false — керування з сайту
boolean backlightState = true;      // Поточний стан підсвітки екрана
boolean lastBacklightState = true;  // Попередній ствн підсвітки

LiquidCrystal_I2C lcd(0x27, 16, 2); // обьекти
DHT dht(DHT_PIN, DHT_TYPE);
void setup() {

Serial.begin(9600);
dht.begin();
pinMode(LED_RED_PIN, OUTPUT);
pinMode(BUTTON_PIN, INPUT_PULLUP);

lcd.init(); // ініціалізація дісплею;
lcd.setCursor(0, 0);
lcd.print("Stream: Pause");
lcd.setCursor(0, 1);
lcd.print("Hello, Ivan :) !");
}

void loop() {

if (Serial.available() > 0) {            // Зчитування даних з Serial port(Messages)
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

if(cmd.startsWith("LCD:")){
  String lcdText = cmd.substring(4);
  if (lcdText.length() > 16) {
    lcdText = lcdText.substring(0, 16);
  }
  lcd.setCursor(0, 1);
  lcd.print("                "); // 16 пробілов
  lcd.setCursor(0, 1);
  lcd.print(lcdText);
}

if(cmd.startsWith("LED:")){
  int intLedState = cmd.substring(4).toInt();
  if(intLedState == 1){
    ledState = true;
  }
  else if(intLedState == 0){
    ledState = false;
  }
}

if(cmd.startsWith("INT:")){
  int interval = cmd.substring(4).toInt();
  if(interval>=1){
    readInterval = (unsigned long)interval * 1000UL;
  }
}
}
if(ledState == true){
digitalWrite(LED_RED_PIN, HIGH);
}
else{
digitalWrite(LED_RED_PIN, LOW);
}

if (backlightState != lastBacklightState) { //управління підсвіткою;
    if (backlightState) {
      lcd.backlight();
    } else {
      lcd.noBacklight();
    }
    lastBacklightState = backlightState;
  }

bool currentButtonState = digitalRead(BUTTON_PIN);
if (lastButtonState == HIGH && currentButtonState == LOW) {
dhtEnabled = !dhtEnabled; // Змінюємо стани;
lcd.setCursor(0, 0);
lcd.print("                ");
lcd.setCursor(0, 0);
if (dhtEnabled) {
//Serial.println(">>> Потік даних DHT11 та Photoresistor: УВІМКНЕНО");
forceRead = true; // Вимагаємо прочитати датчики негайно, не чекаючи readInterval
lcd.print("Stream: ON");
} else {
//Serial.println(">>> Потік даних DHT11 та Photoresistor: ВИМКНЕНО");
lcd.print("Stream: Pause");
}
delay(50);
}
lastButtonState = currentButtonState;
unsigned long currentTime = millis();
if (currentTime - lastLightTime >= LIGHT_INTERVAL) {
  lastLightTime = currentTime;
  // тут зчитування PHOTORESISTOR_PIN, обрахунок lightPercent та блок if (autoBacklight)
  int rawLight = analogRead(PHOTORESISTOR_PIN);
  lightPercent = map(rawLight, 0, 1023, 0, 100); // - переклад у проценти;

  if (autoBacklight) {
      if (lightPercent < 30) {
        backlightState = true; 
      } else if (lightPercent > 35) {
        backlightState = false; 
      }
    }
}
if (forceRead || (currentTime - lastReadTime >= readInterval)) {
forceRead = false;          // Одразу скидаємо прапорець, щоб далі знову працював таймер
lastReadTime = currentTime;
if(dhtEnabled){
float h = dht.readHumidity();
float t = dht.readTemperature(); //- функції для зчитування температури та вологості;

  if (isnan(h) || isnan(t)) {
    //Serial.println("Помилка зчитування! D2");
    lcd.setCursor(0, 0);
    lcd.print("                "); // 16 пробілів стирають тільки верхній рядок
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error D2");
    return;
  }
else{
//Serial.println("_______________________");
Serial.print("{\"temperature\":");
Serial.print(t,1);

Serial.print(",\"humidity\":");
Serial.print(h,0);

Serial.print(",\"light\":");
Serial.print(lightPercent);
Serial.println("}");
//Serial.println("_______________________");
// вивід даних на ЖК-екран
lcd.setCursor(0, 0);
lcd.print("                "); // 16 пробілів стирають тільки верхній рядок
lcd.setCursor(0, 0);
lcd.print("t:");
lcd.print(t, 1);
lcd.print(";h:");
lcd.print(h, 0);
lcd.print(";l:");
lcd.print(lightPercent);
    }
  }
}
}