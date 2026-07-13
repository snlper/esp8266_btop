#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/TomThumb.h> // Подключаем TomThumb для шкалы

#include "icons.h"

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Переменные данных от ПК
int cpuLoad = 0;
int ramLoad = 0;
int gpuLoad = 0;
String hostName = "not connected";

// Буферы для хранения истории графиков
#define GRAPH_WIDTH 57
int cpuHistory[GRAPH_WIDTH];
int gpuHistory[GRAPH_WIDTH];

unsigned long lastDrawTime = 0;
const unsigned long FRAME_DELAY = 100; // Обновление экрана каждые 100 мс

// Функция добавления нового значения в историю графика
void updateHistory(int historyArray[], int newValue) {
  for (int i = 0; i < GRAPH_WIDTH - 1; i++) {
    historyArray[i] = historyArray[i + 1];
  }
  historyArray[GRAPH_WIDTH - 1] = newValue;
}

// Рисует график с заполнением (fill) в указанных координатах
void drawBtopGraph(int x, int y, int h, int historyArray[]) {
  for (int i = 0; i < GRAPH_WIDTH; i++) {
    int graphBarHeight = map(historyArray[i], 0, 100, 1, h);
    if (graphBarHeight > 0) {
      int yStart = y + h - graphBarHeight;
      display.drawFastVLine(x + i, yStart, graphBarHeight, SSD1306_WHITE);
    }
  }
}

// Рисует текстовую шкалу заполнения для RAM
void drawRamBar(int x, int y, int widthChars, int percentage) {
  int filledChars = (percentage * widthChars) / 100;
  display.setCursor(x, y);
  display.print(F("["));
  for (int i = 0; i < widthChars; i++) {
    if (i < filledChars) {
      display.print(F("|")); 
    } else {
      display.print(F(".")); 
    }
  }
  display.print(F("]"));
}

// Функция для вычисления длины числа в символах
int getLength(int value) {
  if (value >= 100) return 3;
  if (value >= 10) return 2;
  return 1;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 5);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); 
  }

  for (int i = 0; i < GRAPH_WIDTH; i++) {
    cpuHistory[i] = 0;
    gpuHistory[i] = 0;
  }

  display.clearDisplay();
  display.setFont(); // Стандартный шрифт
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 25); // Координаты под стандартный шрифт
  display.println(F("btop loading..."));
  display.display();
}

void loop() {
  // 1. Прием данных от ПК
  if (Serial.available() > 0) {
    String inputData = Serial.readStringUntil('\n');
    inputData.trim(); 

    int firstColon = inputData.indexOf(':');
    if (firstColon != -1) {
      int secondColon = inputData.indexOf(':', firstColon + 1);
      if (secondColon != -1) {
        int thirdColon = inputData.indexOf(':', secondColon + 1);
        if (thirdColon != -1) {
          hostName = inputData.substring(0, firstColon);
          cpuLoad = inputData.substring(firstColon + 1, secondColon).toInt();
          ramLoad = inputData.substring(secondColon + 1, thirdColon).toInt();
          gpuLoad = inputData.substring(thirdColon + 1).toInt();

          updateHistory(cpuHistory, cpuLoad);
          updateHistory(gpuHistory, gpuLoad);
        }
      }
    }
  }

  // 2. Отрисовка интерфейса по таймеру
  unsigned long currentTime = millis();
  if (currentTime - lastDrawTime >= FRAME_DELAY) {
    lastDrawTime = currentTime;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // ==========================================================
    // ВЕЗДЕ ИСПОЛЬЗУЕМ СТАНДАРТНЫЙ ШРИФТ
    // ==========================================================
    display.setFont(); // Сброс на стандартный шрифт

    // СЕКЦИЯ SYSTEM / HOSTNAME
    display.drawRoundRect(0, 0, 128, 14, 3, SSD1306_WHITE);
    int titleX = (128 - (hostName.length() * 6)) / 2;
    display.setCursor(titleX > 4 ? titleX : 4, 3);
    display.print(hostName);

    // СЕКЦИЯ CPU
    display.drawRoundRect(0, 16, 63, 33, 2, SSD1306_WHITE);
    display.drawBitmap(4, 18, cpu_icon, 8, 8, SSD1306_WHITE); // Иконка CPU
    // Выравнивание по правому краю рамки (X=59 — крайняя точка для текста)
    // Вычитаем длину числа + знак % (всего символов: length + 1), умноженную на 6 пикселей
    int cpuTextX = 60 - ((getLength(cpuLoad) + 1) * 6);
    display.setCursor(cpuTextX, 19);
    display.print(cpuLoad);
    display.print(F("%"));
    drawBtopGraph(3, 28, 18, cpuHistory);

    // СЕКЦИЯ GPU
    display.drawRoundRect(65, 16, 63, 33, 2, SSD1306_WHITE);
    display.drawBitmap(68, 18, gpu_icon, 16, 8, SSD1306_WHITE); // Иконка GPU
    // Выравнивание по правому краю рамки (X=124 — крайняя точка для текста)
    int gpuTextX = 125 - ((getLength(gpuLoad) + 1) * 6);
    display.setCursor(gpuTextX, 19);
    display.print(gpuLoad);
    display.print(F("%"));
    drawBtopGraph(68, 28, 18, gpuHistory);

    // СЕКЦИЯ RAM
    display.drawRoundRect(0, 51, 128, 13, 3, SSD1306_WHITE);
    display.setCursor(4, 53); 
    display.print(F("mem"));
    // Выравнивание перед шкалой TomThumb. Крайняя точка для текста — X=46
    int ramTextX = 46 - ((getLength(ramLoad) + 1) * 6);
    display.setCursor(ramTextX, 54);
    display.print(ramLoad);
    display.print(F("%"));

    // ==========================================================
    // ДЛЯ ШКАЛЫ ПЕРЕКЛЮЧАЕМСЯ НА TOMTHUMB
    // ==========================================================
    display.setFont(&TomThumb); 
    drawRamBar(48, 60, 34, ramLoad); 

    display.display();
  }
}
