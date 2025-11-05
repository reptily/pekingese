#include "Pekingese.h"

#define SERIAL_SPEED_RATE 9600
#define PIN_WATER_PUMP 13 // d7
#define PIN_LED 2 // Встроенный LED на ESP8266
#define PIN_RESET_BUTTON 12 // d6

// Создание объекта Pekingese с настройками:
// - Имя AP сети: "WaterPump-AP"
// - Пароль AP: "mypassword"
// - LED пин: 2 (встроенный LED)
// - Кнопка сброса: 12
Pekingese wifi("WaterPump-AP", "mypassword", PIN_LED, PIN_RESET_BUTTON);

void setup() 
{
  Serial.begin(SERIAL_SPEED_RATE);
  
  pinMode(PIN_WATER_PUMP, OUTPUT);
  
  // Инициализация Pekingese (автоматически настроит WiFi и пины)
  wifi.begin();
  
  // Добавление дополнительных маршрутов
  wifi.routes().on("/run/water_pump", handleRunWaterPump);
  wifi.routes().on("/status", handleStatus);
}

void loop() 
{
  // Обработка HTTP запросов, кнопки сброса и LED
  // Все управляется внутри класса Pekingese
  wifi.loop();
}

// Дополнительный маршрут для управления водяным насосом
void handleRunWaterPump() 
{
  ESP8266WebServer& server = wifi.routes();
  
  if (server.method() != HTTP_POST) {
    String message = "{\"status\":\"ERROR\", \"message\": \"Request method not supported\"}";
    server.send(400, "application/json", message);
    return;
  }
  
  String microsecondsParam = server.arg("microseconds");
  int microseconds = microsecondsParam.toInt();
  
  if (microseconds <= 0) {
    String message = "{\"status\":\"ERROR\", \"message\": \"Microseconds must be greater than 0\"}";
    server.send(400, "application/json", message);
    return;
  }
  
  String message = "{\"status\":\"OK\", \"message\": \"Water pump start\"}";
  server.send(200, "application/json", message);
  
  runWaterPump(microseconds);
}

void runWaterPump(int microseconds) 
{
  digitalWrite(PIN_WATER_PUMP, HIGH);
  delay(microseconds); 
  digitalWrite(PIN_WATER_PUMP, LOW); 
}

// Дополнительный маршрут для получения статуса
void handleStatus()
{
  ESP8266WebServer& server = wifi.routes();
  
  String json = "{";
  json += "\"status\":\"OK\",";
  json += "\"connected\":" + String(wifi.isConnected() ? "true" : "false") + ",";
  json += "\"ap_mode\":" + String(wifi.isAPMode() ? "true" : "false") + ",";
  json += "\"ip\":\"" + wifi.getLocalIP() + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

