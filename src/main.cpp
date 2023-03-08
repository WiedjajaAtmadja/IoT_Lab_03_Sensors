/*
Program: demo gpio 2 led, 1 interrupt switch, luxMeter and DHT11
  - program will print Temperature, Humidity and Light every 2 secs
  - detect push button for counting, detect bounching if exists

 Devices:
- Led Red:    D4 (active high, Anode Led to IO02 via R (68-220) Ohm, Cathode: GND)
- Led Yellow: D5 (active high, Anode Led to IO12 via R (68-220) Ohm, Cathode: GND)
- Led Green:  D18 (active high, Anode Led to IO12 via R (68-220) Ohm, Cathode: GND)
- PushButton (PIN_SW): D23 (active Low, add pull up 10K to 3.3V)
- DHT11: D19, add pullup 10K to VCC
- BH1750: I2C (SDA: D21, SCL:D22; add pullup 10K to 3.3V), 
  default addr: 0x23, mode: continuous high resolution mode
*/
#include <Arduino.h>
#include <Ticker.h>
#include <DHTesp.h>
#include <BH1750.h>
#define LED_GREEN  4
#define LED_YELLOW 5
#define LED_RED    18
#define PUSH_BUTTON 23
#define DHT_PIN 19

int g_nCount=0;
Ticker timer1Sec;
Ticker timerLedBuiltinOff;
Ticker timerLedRedOff;
Ticker timerYellowOff;
Ticker timerReadSensor;

DHTesp dht;
BH1750 lightMeter;

void OnTimer1Sec() {
  digitalWrite(LED_BUILTIN, HIGH);
  timerLedBuiltinOff.once_ms(50, []() {
    digitalWrite(LED_BUILTIN, LOW);
  });
}

volatile bool g_fUpdated = false;
IRAM_ATTR void isrPushButton()
{
  g_nCount++;
  g_fUpdated = true;
}

void OnReadSensor()
{
  float fHumidity = dht.getHumidity();
  float fTemperature = dht.getTemperature();
  float lux = lightMeter.readLightLevel();
  Serial.printf("Humidity: %.2f, Temperature: %.2f, Light: %.2f\n", 
    fHumidity, fTemperature, lux);
  digitalWrite(LED_YELLOW, (fHumidity > 80)?HIGH:LOW);
}

void setup() {
  // init Leds
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  // init push button interrupt
  pinMode(PUSH_BUTTON, INPUT_PULLUP);
  attachInterrupt(PUSH_BUTTON, isrPushButton, FALLING);

  Serial.begin(115200);

  dht.setup(DHT_PIN, DHTesp::DHT11);

  Wire.begin();
  lightMeter.begin();

  timer1Sec.attach_ms(1000, OnTimer1Sec);
  timerReadSensor.attach_ms(2000, OnReadSensor);
  Serial.println("System ready");
}

int g_nLastCount = 0;
void loop() {
  if (g_fUpdated) {
    g_fUpdated = false;
    Serial.printf("Count: %d\n", g_nCount);
    if (g_nCount - g_nLastCount>1) {
      Serial.printf("Bounching detected! Count: %d, LastCount: %d\n", g_nCount, g_nLastCount);
      digitalWrite(LED_RED, HIGH);
      timerLedRedOff.once_ms(50, []() {
        digitalWrite(LED_RED, LOW);
      });
    }
    g_nLastCount = g_nCount;
  }
}