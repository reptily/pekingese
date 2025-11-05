#ifndef PEKINGESE_H
#define PEKINGESE_H

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include "LittleFS.h"

#define FILE_SETTINGS "/settings.cfg"

struct SettingsStruct {
  char ssid[36];
  char password[36];
};

class Pekingese {
public:
  // Конструктор с настройками
  Pekingese(
    const char* apSSID = "Pekingese-AP",
    const char* apPassword = "12345678",
    int ledPin = 2,
    int resetButtonPin = -1,
    int serverPort = 80,
    unsigned long retryConnect = 5000,
    unsigned long ledBlinkInterval = 500
  );
  
  void begin();
  void loop();
  void restart();
  
  // Метод для добавления пользовательских маршрутов
  ESP8266WebServer& routes();
  
  // Настройка пинов
  void setLedPin(int pin);
  void setResetButtonPin(int pin);
  
  // Геттеры
  bool isConnected();
  String getLocalIP();
  bool isAPMode();
  
private:
  ESP8266WebServer _server;
  SettingsStruct _settings;
  IPAddress _localIP;
  IPAddress _gateway;
  IPAddress _subnet;
  
  // Настраиваемые параметры
  String _apSSID;
  String _apPassword;
  int _ledPin;
  int _resetButtonPin;
  int _serverPort;
  unsigned long _retryConnect;
  unsigned long _ledBlinkInterval;
  bool _isAPMode;
  
  // Управление LED
  unsigned long _lastBlinkTime;
  bool _ledState;
  
  // Инициализация файловой системы
  void _initFS();
  
  // Проверка и загрузка настроек
  bool _existsSettings();
  void _initSettings();
  void _setSettings(String ssid, String password);
  void _resetSettings();
  
  // WiFi управление
  void _startAP();
  void _startSTA(String ssid, String password);
  
  // Приватные стандартные маршруты
  void _setupDefaultRoutes();
  void _handleTest();
  void _handleReset();
  void _handleWifiUpdateSetting();
  void _handleWifiResetSetting();
  
  // HTTP сервер
  void _createHttpServer();
  void _listen();
  
  // LED управление
  void _updateLed();
  void _handleResetButton();
  
  // Дебаг
  void _debug(String message);
};

#endif

