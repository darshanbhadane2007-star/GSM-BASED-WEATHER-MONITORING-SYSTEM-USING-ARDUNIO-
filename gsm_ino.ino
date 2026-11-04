


#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP085.h>

// ------------ SENSOR CONFIGURATION ------------

#define DHTPIN 8
#define DHTTYPE DHT11

#define LDR_PIN A1
#define RAIN_A A0
#define RAIN_D 7
#define BUZZER_PIN 13

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// GSM: Arduino RX = 11, TX = 12
SoftwareSerial gsm(11, 12);

const char PHONE_NUMBER[] = "+919022334513";

bool bmpAvailable = false;
bool buzzerState = false;
bool smsPending = false;

unsigned long buzzerTimer = 0;
unsigned long displayTimer = 0;

const unsigned long BUZZER_ON_TIME = 500;
const unsigned long BUZZER_OFF_TIME = 500;
const unsigned long DISPLAY_INTERVAL = 3000;

String incomingLine = "";

// ------------ WEATHER PREDICTION ------------

String predictWeather(float temp, float hum, int rainVal) {

  if (rainVal < 500 && hum > 70) {
    return "Rain Soon";
  }
  else if (temp > 30 && hum < 40) {
    return "Sunny";
  }
  else {
    return "Cloudy";
  }
}

// ------------ LCD FUNCTION ------------

void printLCDLine(int row, String message) {

  lcd.setCursor(0, row);

  if (message.length() > 16) {
    message = message.substring(0, 16);
  }

  while (message.length() < 16) {
    message += " ";
  }

  lcd.print(message);
}

// ------------ GSM SMS FUNCTION ------------

void sendSMS(String msg) {

  gsm.println("AT+CMGF=1");
  delay(1000);

  gsm.print("AT+CMGS=\"");
  gsm.print(PHONE_NUMBER);
  gsm.println("\"");

  delay(1500);

  gsm.print(msg);
  gsm.write(26);

  delay(5000);

  Serial.println("SMS command sent");
}

// ------------ BUZZER FUNCTION ------------

void buzzerAlert(bool rainDetected) {

  if (!rainDetected) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = false;
    return;
  }

  unsigned long currentTime = millis();

  if (buzzerState) {

    if (currentTime - buzzerTimer >= BUZZER_ON_TIME) {
      digitalWrite(BUZZER_PIN, LOW);
      buzzerState = false;
      buzzerTimer = currentTime;
    }

  } else {

    if (currentTime - buzzerTimer >= BUZZER_OFF_TIME) {
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerState = true;
      buzzerTimer = currentTime;
    }
  }
}

// ------------ GSM MESSAGE CHECK ------------

void checkSMS() {

  while (gsm.available()) {

    char c = gsm.read();

    if (c == '\n' || c == '\r') {

      if (incomingLine.indexOf("HELLO") >= 0 ||
          incomingLine.indexOf("hello") >= 0) {

        smsPending = true;
      }

      incomingLine = "";
    }
    else {

      if (incomingLine.length() < 100) {
        incomingLine += c;
      }
    }
  }
}

// ------------ SETUP ------------

void setup() {

  Serial.begin(9600);
  gsm.begin(9600);

  dht.begin();

  lcd.init();
  lcd.backlight();

  pinMode(RAIN_D, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);

  // BMP180 initialization
  bmpAvailable = bmp.begin();

  if (bmpAvailable) {
    Serial.println("BMP180 detected");
  }
  else {
    Serial.println("BMP180 not detected");
  }

  // GSM initialization
  gsm.println("AT");
  delay(1000);

  gsm.println("AT+CMGF=1");
  delay(1000);

  gsm.println("AT+CNMI=1,2,0,0,0");
  delay(1000);

  lcd.clear();

  printLCDLine(0, "Weather Monitor");
  printLCDLine(1, "System Ready");

  delay(2000);

  lcd.clear();

  Serial.println("Weather System Started");
}

// ------------ MAIN LOOP ------------

void loop() {

  // Read sensors
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  int ldrVal = analogRead(LDR_PIN);
  int rainAnalog = analogRead(RAIN_A);
  int rainDigital = digitalRead(RAIN_D);

  bool rainDetected = (rainDigital == LOW);

  // Check DHT sensor readings
  if (isnan(temp) || isnan(hum)) {

    Serial.println("DHT11 reading error");

    printLCDLine(0, "DHT Sensor Error");
    printLCDLine(1, "Check Connection");

    checkSMS();
    delay(500);
    return;
  }

  // Read BMP180
  float pressure = 0;
  float altitude = 0;

  if (bmpAvailable) {

    pressure = bmp.readPressure() / 100.0;
    altitude = bmp.readAltitude();
  }

  // Weather conditions
  String light = (ldrVal < 500) ? "Dark" : "Bright";

  String rainStatus = rainDetected ? "Rain" : "No Rain";

  String prediction = predictWeather(
    temp, hum, rainAnalog
  );

  String rainForecast;

  if (rainDetected) {
    rainForecast = "Rain Detected";
  }
  else if (prediction == "Rain Soon") {
    rainForecast = "Rain Expected";
  }
  else {
    rainForecast = "No Rain Expected";
  }

  // Weather report
  String weatherUpdate =
    "Temp:" + String(temp, 1) + "C " +
    "Hum:" + String(hum, 0) + "% " +
    "Pressure:" +
    (bmpAvailable ? String(pressure, 1) : String("NA")) +
    "hPa " +
    "Light:" + light + " " +
    "Condition:" + (rainDetected ? "Rain" : prediction);

  String forecast =
    "Forecast:" + rainForecast +
    " Temp:" + String(temp + 2, 1) + "C";

  // Serial monitor output
  Serial.println("------------------------");
  Serial.println(weatherUpdate);
  Serial.println(forecast);

  if (bmpAvailable) {
    Serial.print("Altitude: ");
    Serial.print(altitude);
    Serial.println(" meters");
  }

  // Buzzer control
  buzzerAlert(rainDetected);

  // LCD display update
  unsigned long currentTime = millis();

  if (currentTime - displayTimer >= DISPLAY_INTERVAL) {

    displayTimer = currentTime;

    static bool displayPage = false;

    lcd.clear();

    if (!displayPage) {

      printLCDLine(0,
        "T:" + String(temp, 1) + "C H:" +
        String(hum, 0) + "%");

      printLCDLine(1,
        "P:" +
        (bmpAvailable ? String(pressure, 0) : String("NA")) +
        "hPa " + rainStatus);
    }
    else {

      printLCDLine(0, "Weather Forecast");

      printLCDLine(1, rainForecast);
    }

    displayPage = !displayPage;
  }

  // Check incoming GSM messages
  checkSMS();

  // Send weather report when HELLO is received
  if (smsPending) {

    smsPending = false;

    sendSMS(
      "Weather Report:\n" +
      weatherUpdate + "\n" +
      forecast
    );
  }

  delay(100);
}
