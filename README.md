# Класс Pekingese

Класс для управления WiFi соединением ESP8266 с автоматическим переключением между режимами Access Point (AP) и Station (STA).

## Возможности

- **Автоматическое управление WiFi**: если настройки WiFi не найдены, создается точка доступа для настройки
- **Режим AP**: создание собственной WiFi сети для настройки
- **Режим STA**: подключение к существующей WiFi сети
- **HTTP сервер**: встроенный веб-сервер с стандартными маршрутами
- **Приватные стандартные маршруты**: базовые маршруты управления скрыты от пользователя
- **Публичный метод routes()**: для добавления пользовательских маршрутов
- **Настраиваемые параметры**: имя и пароль AP сети, пины LED и кнопки сброса
- **Индикация LED**: автоматическое управление светодиодом (мигает в AP, горит при подключении)
- **Кнопка сброса**: автоматическая обработка кнопки сброса настроек

## Стандартные маршруты (приватные)

Эти маршруты автоматически настраиваются классом:

- `GET /test` - проверка работоспособности
- `GET /restart` - перезагрузка устройства
- `POST /wifi/update_setting` - обновление настроек WiFi (параметры: ssid, password)
- `DELETE /wifi/reset_setting` - сброс настроек WiFi

## Использование

### Базовый пример

```cpp
#include "Pekingese.h"

// Использование с параметрами по умолчанию
Pekingese wifi;

void setup() {
  Serial.begin(9600);
  
  // Инициализация (автоматически настроит WiFi)
  wifi.begin();
  
  // Добавление пользовательских маршрутов
  wifi.routes().on("/my/custom/route", handleCustomRoute);
}

void loop() {
  wifi.loop();
}

void handleCustomRoute() {
  ESP8266WebServer& server = wifi.routes();
  server.send(200, "text/plain", "Hello World!");
}
```

### Пример с настройками

```cpp
#include "Pekingese.h"

// Настройка AP сети, LED и кнопки сброса
// Pekingese(AP_SSID, AP_PASSWORD, LED_PIN, RESET_BUTTON_PIN)
Pekingese wifi("MyDevice-AP", "mypassword123", 2, 12);

void setup() {
  Serial.begin(9600);
  wifi.begin();
  
  wifi.routes().on("/status", handleStatus);
}

void loop() {
  wifi.loop();  // Автоматически обрабатывает LED и кнопку
}

void handleStatus() {
  ESP8266WebServer& server = wifi.routes();
  String json = "{\"connected\":" + String(wifi.isConnected() ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}
```

### Пример с несколькими маршрутами

```cpp
void setup() {
  Serial.begin(9600);
  wifi.begin();
  
  // Добавление нескольких маршрутов
  wifi.routes().on("/api/status", handleStatus);
  wifi.routes().on("/api/data", HTTP_POST, handlePostData);
  wifi.routes().on("/api/sensor", handleSensor);
}

void handleStatus() {
  ESP8266WebServer& server = wifi.routes();
  String json = "{\"status\":\"OK\",\"connected\":";
  json += wifi.isConnected() ? "true" : "false";
  json += "}";
  server.send(200, "application/json", json);
}
```

## API

### Конструктор

```cpp
Pekingese(
  const char* apSSID = "Pekingese-AP",
  const char* apPassword = "12345678",
  int ledPin = 2,
  int resetButtonPin = -1,
  int serverPort = 80,
  unsigned long retryConnect = 5000,
  unsigned long ledBlinkInterval = 500
)
```

**Параметры:**
- `apSSID` - имя WiFi сети в режиме AP (по умолчанию: "Pekingese-AP")
- `apPassword` - пароль AP сети (по умолчанию: "12345678")
- `ledPin` - пин светодиода индикации (по умолчанию: 2, -1 для отключения)
- `resetButtonPin` - пин кнопки сброса (по умолчанию: -1 для отключения)
- `serverPort` - порт HTTP сервера (по умолчанию: 80)
- `retryConnect` - интервал повтора подключения к WiFi в мс (по умолчанию: 5000)
- `ledBlinkInterval` - интервал мигания LED в мс (по умолчанию: 500)

### Публичные методы

#### `void begin()`
Инициализирует WiFi соединение, LED и кнопку сброса. Автоматически определяет режим работы:
- Если настройки не найдены → запускает AP режим (диод мигает)
- Если настройки найдены → подключается к WiFi сети (диод горит)

#### `void loop()`
Обрабатывает:
- HTTP запросы
- Обновление MDNS
- Состояние LED
- Кнопку сброса

Должен вызываться в основном цикле `loop()`.

#### `void restart()`
Перезагружает устройство через 3 секунды.

#### `ESP8266WebServer& routes()`
Возвращает ссылку на объект сервера для добавления пользовательских маршрутов.

**Пример:**
```cpp
wifi.routes().on("/custom", handleCustom);
```

#### `void setLedPin(int pin)`
Изменяет пин LED в процессе работы. Укажите -1 для отключения.

#### `void setResetButtonPin(int pin)`
Изменяет пин кнопки сброса в процессе работы. Укажите -1 для отключения.

#### `bool isConnected()`
Проверяет, подключено ли устройство к WiFi сети.

**Возвращает:** `true` если подключено, `false` если нет.

#### `String getLocalIP()`
Возвращает IP адрес устройства.

**Возвращает:** IP адрес в виде строки (AP IP или STA IP в зависимости от режима).

#### `bool isAPMode()`
Проверяет, работает ли устройство в режиме Access Point.

**Возвращает:** `true` если в режиме AP, `false` если в режиме STA.

## Настройки по умолчанию

- **Имя AP сети**: `Pekingese-AP` (настраивается в конструкторе)
- **Пароль AP**: `12345678` (настраивается в конструкторе)
- **AP IP адрес**: `192.168.1.1`
- **Порт HTTP сервера**: `80` (настраивается в конструкторе)
- **LED пин**: `2` (настраивается в конструкторе)
- **Кнопка сброса**: отключена (настраивается в конструкторе)
- **Интервал повтора подключения**: `5000ms` (настраивается в конструкторе)
- **Интервал мигания LED**: `500ms` (настраивается в конструкторе)
- **Файл настроек**: `/settings.cfg` (LittleFS)

## Индикация LED

Светодиод автоматически показывает состояние устройства:

- **Мигает** - режим Access Point (нужна настройка WiFi)
- **Горит** - подключено к WiFi сети
- **Не горит** - нет подключения к WiFi

Чтобы отключить LED индикацию, укажите `-1` в конструкторе:
```cpp
Pekingese wifi("MyAP", "password", -1);  // LED отключен
```

## Настройка WiFi через API

1. Подключитесь к AP сети устройства (имя сети указано в конструкторе, по умолчанию: `Pekingese-AP`)
2. Отправьте POST запрос на `/wifi/update_setting` с параметрами:
   - `ssid` - имя вашей WiFi сети
   - `password` - пароль от WiFi сети
3. Перезагрузите устройство
4. Устройство подключится к вашей WiFi сети (диод перестанет мигать и будет гореть постоянно)

**Пример curl:**
```bash
curl -X POST http://192.168.1.1/wifi/update_setting \
  -d "ssid=MyWiFi&password=MyPassword123"
```

## Кнопка сброса

Если в конструкторе указан пин кнопки сброса, класс автоматически обрабатывает нажатие:
- При нажатии кнопки сбрасываются настройки WiFi
- Устройство перезагружается
- После перезагрузки запускается режим AP (диод мигает)

Кнопка подключается с `INPUT_PULLUP`, то есть должна замыкать пин на GND.

## Структура файлов

- `Pekingese.h` - заголовочный файл класса
- `Pekingese.cpp` - реализация класса
- `example_usage.ino` - пример использования

## Требования

- ESP8266 библиотеки:
  - `ESP8266WebServer`
  - `ESP8266mDNS`
  - `ESP8266WiFi`
  - `LittleFS`

## Использование в других проектах

Просто скопируйте файлы `Pekingese.h` и `Pekingese.cpp` в ваш проект и включите заголовочный файл:

```cpp
#include "Pekingese.h"
```

