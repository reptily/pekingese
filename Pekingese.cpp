#include "Pekingese.h"

#define DEBUG true
#define MSECONDS_AFTER_RESTART 3000

Pekingese::Pekingese(
  const char* apSSID,
  const char* apPassword,
  int ledPin,
  int resetButtonPin,
  int serverPort,
  unsigned long retryConnect,
  unsigned long ledBlinkInterval
) 
  : _server(serverPort),
    _localIP(192, 168, 1, 1),
    _gateway(192, 168, 1, 2),
    _subnet(255, 255, 255, 0),
    _apSSID(apSSID),
    _apPassword(apPassword),
    _ledPin(ledPin),
    _resetButtonPin(resetButtonPin),
    _serverPort(serverPort),
    _retryConnect(retryConnect),
    _ledBlinkInterval(ledBlinkInterval),
    _isAPMode(false),
    _lastBlinkTime(0),
    _ledState(false)
{
  strcpy(_settings.ssid, "wifi");
  strcpy(_settings.password, "12345678");
}

void Pekingese::begin() 
{
  // Инициализация пинов
  if (_ledPin >= 0) {
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW);
  }
  
  if (_resetButtonPin >= 0) {
    pinMode(_resetButtonPin, INPUT_PULLUP);
  }
  
  _initFS();
  
  if (!_existsSettings()) {
    _debug("File settings not found");
    _startAP();
    return;
  }
  
  _initSettings();
  _startSTA(_settings.ssid, _settings.password);
}

void Pekingese::loop() 
{
  _handleResetButton();
  _updateLed();
  _listen();
}

void Pekingese::restart() 
{
  delay(MSECONDS_AFTER_RESTART);
  ESP.restart();
}

ESP8266WebServer& Pekingese::routes() 
{
  return _server;
}

bool Pekingese::isConnected() 
{
  return WiFi.status() == WL_CONNECTED;
}

String Pekingese::getLocalIP() 
{
  if (WiFi.getMode() == WIFI_AP) {
    return WiFi.softAPIP().toString();
  } else {
    return WiFi.localIP().toString();
  }
}

bool Pekingese::isAPMode()
{
  return _isAPMode;
}

void Pekingese::setLedPin(int pin)
{
  if (_ledPin >= 0) {
    digitalWrite(_ledPin, LOW);
  }
  
  _ledPin = pin;
  
  if (_ledPin >= 0) {
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW);
  }
}

void Pekingese::setResetButtonPin(int pin)
{
  _resetButtonPin = pin;
  
  if (_resetButtonPin >= 0) {
    pinMode(_resetButtonPin, INPUT_PULLUP);
  }
}

// Приватные методы

void Pekingese::_initFS() 
{
  if (!LittleFS.begin()) {
    _debug("ERROR FS");
  } else {
    _debug("FS init OK");
  }
}

bool Pekingese::_existsSettings() 
{
  return LittleFS.exists(FILE_SETTINGS);
}

void Pekingese::_initSettings() 
{
  File fileSettings = LittleFS.open(FILE_SETTINGS, "r");
  
  if (fileSettings) {
    uint16_t bytesRead = fileSettings.read((byte *) &_settings, sizeof(_settings));
    _debug("read file settings ssid:" + (String)_settings.ssid + " password:" + (String)_settings.password);
    fileSettings.close();
  }
}

void Pekingese::_setSettings(String ssid, String password) 
{
  File fileSettings = LittleFS.open(FILE_SETTINGS, "w+");
  
  if (!fileSettings) {
    _debug("file settings open failed");
    String message = "{\"status\":\"ERROR\", \"message\": \"File settings open failed\"}";
    _server.send(500, "application/json", message);
    return;
  }
  
  ssid.toCharArray(_settings.ssid, 36);
  password.toCharArray(_settings.password, 36);
  
  fileSettings.write((byte *) &_settings, sizeof(_settings));
  fileSettings.close();
  
  _debug("write to settings ssid:" + ssid + " password:" + password);
}

void Pekingese::_resetSettings() 
{
  LittleFS.remove(FILE_SETTINGS);
  _debug("Reset settings");
}

void Pekingese::_startAP() 
{
  _debug("Starting AP");
  _isAPMode = true;
  WiFi.softAPConfig(_localIP, _gateway, _subnet);
  WiFi.softAP(_apSSID.c_str(), _apPassword.c_str());
  
  _debug("AP SSID: " + _apSSID);
  _debug("AP IP: " + WiFi.softAPIP().toString());
  
  _createHttpServer();
}

void Pekingese::_startSTA(String ssid, String password) 
{
  _isAPMode = false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(_retryConnect);
    yield();
    _debug(".");
  }
  
  _debug("Connected to " + ssid);
  _debug("IP address: " + WiFi.localIP().toString());
  
  if (MDNS.begin("esp8266")) {
    _debug("MDNS responder started");
  }
  
  _createHttpServer();
}

void Pekingese::_setupDefaultRoutes() 
{
  _server.on("/test", [this]() { this->_handleTest(); });
  _server.on("/restart", [this]() { this->_handleReset(); });
  _server.on("/wifi/update_setting", [this]() { this->_handleWifiUpdateSetting(); });
  _server.on("/wifi/reset_setting", [this]() { this->_handleWifiResetSetting(); });
}

void Pekingese::_handleTest() 
{
  String message = "{\"status\":\"OK\"}";
  _server.send(200, "application/json", message);
}

void Pekingese::_handleReset() 
{
  if (_server.method() != HTTP_GET) {
    String message = "{\"status\":\"ERROR\", \"message\": \"Request method not supported\"}";
    _server.send(400, "application/json", message);
    return;
  }
  
  String message = "{\"status\":\"OK\", \"message\": \"Device is reset\"}";
  _server.send(200, "application/json", message);
  
  restart();
}

void Pekingese::_handleWifiUpdateSetting() 
{
  if (_server.method() != HTTP_POST) {
    String message = "{\"status\":\"ERROR\", \"message\": \"Request method not supported\"}";
    _server.send(400, "application/json", message);
    return;
  }
  
  String ssid = _server.arg("ssid");
  String password = _server.arg("password");
  
  _setSettings(ssid, password);
  
  String message = "{\"status\":\"OK\", \"message\": \"Settings is saved\"}";
  _server.send(200, "application/json", message);
}

void Pekingese::_handleWifiResetSetting() 
{
  if (_server.method() != HTTP_DELETE) {
    String message = "{\"status\":\"ERROR\", \"message\": \"Request method not supported\"}";
    _server.send(400, "application/json", message);
    return;
  }
  
  _resetSettings();
  
  String message = "{\"status\":\"OK\", \"message\": \"Settings is reset\"}";
  _server.send(200, "application/json", message);
}

void Pekingese::_createHttpServer() 
{
  _setupDefaultRoutes();
  _server.begin();
  _debug("HTTP server started");
}

void Pekingese::_listen() 
{
  _server.handleClient();
  MDNS.update();
}

void Pekingese::_updateLed()
{
  if (_ledPin < 0) {
    return;
  }
  
  if (_isAPMode) {
    // Режим AP - мигает диод
    unsigned long currentTime = millis();
    if (currentTime - _lastBlinkTime >= _ledBlinkInterval) {
      _lastBlinkTime = currentTime;
      _ledState = !_ledState;
      digitalWrite(_ledPin, _ledState ? HIGH : LOW);
    }
  } else if (isConnected()) {
    // Подключен к WiFi - диод горит
    digitalWrite(_ledPin, HIGH);
  } else {
    // Не подключен - диод не горит
    digitalWrite(_ledPin, LOW);
  }
}

void Pekingese::_handleResetButton()
{
  if (_resetButtonPin < 0) {
    return;
  }
  
  // Кнопка нажата (LOW при INPUT_PULLUP)
  if (digitalRead(_resetButtonPin) == LOW) {
    _debug("Reset button pressed");
    _resetSettings();
    restart();
  }
}

void Pekingese::_debug(String message) 
{
  if (DEBUG) {
    Serial.println(message);
  }
}

