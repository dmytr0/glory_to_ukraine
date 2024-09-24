#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#define DEBUG 1

#define LED_BUILTIN 8

// OLED дисплей налаштування
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1  // Немає reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Підключення I2C
#define I2C_SDA 6
#define I2C_SCL 7

// Параметри Serial для RF Power Meter
#define RX_PIN 10
#define SERIAL_BAUD_RATE 9600

// Для зберігання останніх 10 вимірів
float powerMeasurements[10];
int measurementIndex = 0;
bool bufferFilled = false;
int ledState[] = { 0, 150, 200, 225, 255 };


int state = 0;
void debugByLedState() {
  analogWrite(LED_BUILTIN, ledState[state]); 
  delay(200);
  if (state < 4) {
    state++;
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  debugByLedState();            ////////////////////////////////////////////////////

  // Ініціалізація серійного порту
#ifdef DEBUG
  Serial.begin(SERIAL_BAUD_RATE);
  debug(F("serial Started"));

#endif

  debugByLedState();            ////////////////////////////////////////////////////
  debug(F("serial1"));

  Serial1.begin(SERIAL_BAUD_RATE, SERIAL_8N1, RX_PIN, -1);
  debug(F("serial1 started"));
 

  debugByLedState();            ////////////////////////////////////////////////////

  // Ініціалізація дисплею
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    debug(F("SSD1306 initialization failed"));
    for (;;)
      ;
  }

  debugByLedState();            ////////////////////////////////////////////////////

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.fillScreen(SSD1306_WHITE);

  debugByLedState();            ////////////////////////////////////////////////////
}

String rline1 = "88888";
String cline1 = "";
String rline2 = "88888";
String cline2 = "";

void loop() {
  readSerial();
  show();
}

void readSerial() {
  if (Serial1.available() > 0) {
    String data = Serial1.readStringUntil('$');
    data.trim();
    if (data.length() == 0) return;

    debug("Data :" + data);

    //Парсинг значення dBm
    int dBmIndex = data.indexOf("dBm");
    if (dBmIndex != -1) {
      String dBmString = data.substring(0, dBmIndex);
      dBmString.trim();
      if (dBmString.length() == 0) return;
      debug("dBmString :" + dBmString);
      float dBm = dBmString.toFloat();

      // Конвертація dBm у мілівати
      float powerMilliwatts = pow(10, dBm / 10);

      // Додавання до масиву вимірів
      powerMeasurements[measurementIndex] = powerMilliwatts;
      measurementIndex = (measurementIndex + 1) % 10;
      if (measurementIndex == 0) bufferFilled = true;

      // Обчислення середнього значення
      float averagePower = 0;
      int count = bufferFilled ? 10 : measurementIndex;
      for (int i = 0; i < count; i++) {
        averagePower += powerMeasurements[i];
      }
      averagePower /= count;

      rline2 = String((int)round(pow(10, dBm / 10)));  // Округлює потужність
      rline1 = String((int)round(averagePower));       // Округлює середнє значення

      debug(rline1);
      debug(rline2);
    }
  }
}

void debug(String msg) {
#ifdef DEBUG
  Serial.println(msg);
#endif
}

void show() {
  if (cline1 != rline1 || rline2 != cline2) {
    display.clearDisplay();
    display.drawLine(60, 0, 60, 32, SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(rline1);
    display.setCursor(getYPosition(), 0);
    display.print(rline2);

    display.setTextSize(1, 1);

    display.setCursor(16, 22);
    display.print(F("avg"));
    display.setCursor(85, 22);
    display.print(F("mW"));

    display.display();
    cline1 = rline1;
    cline2 = rline2;
  }
}


uint8_t getYPosition() {
  return 124 - (rline2.length() * 12);
}
