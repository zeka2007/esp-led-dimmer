#include <Arduino.h>

#include <GyverNTP.h>
#include <GyverDBFile.h>
#include <LittleFS.h>

#define ALARMS_COUNT 3 // количестов будильников

#include "strip.h"
#include "config.h"
#include "alarms.h"

Alarm alarmControl(&NTP);

GyverDBFile db(&LittleFS, "/data.db");

#include <SettingsESP.h>
SettingsESP sett("Led strip", &db);

#define LED_PIN D2 // пин ленты
LedStrip strip(LED_PIN);


uint16_t addBrIn; // добавлять по одной единице яркости каждые n милисекунд

uint16_t ledBr = 255;
bool ledState = false;
bool effectsState = false;
bool autoOffWaiting = false;

uint32_t timeTemp;

uint32_t alarmLocalData[ALARMS_COUNT][alarmDBData::ITEMS_COUNT];

DB_KEYS(
    kk,
    wifi_ssid,
    wifi_pass,
    alarm_auto_off,
    alarm_auto_off_period,
    on_before_alarm_period,
    time_zone,
    apply);

void startAP() {
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("LED STRIP AP");
}

void build(sets::Builder& b) {
    {
      sets::Group g1(b, "Общие настройки");
      
      if (b.Switch("ledState"_h, "Включить ленту", &ledState) || b.Slider("ledBr"_h, "Яркость", 0, 255, 1, Text(), &ledBr)) {
        autoOffWaiting = false;
      }
    }

    {
      sets::Group g3(b, "Будильник");
      if (b.Number(kk::time_zone, "Часовой пояс")) {
        NTP.setGMT(b.build.value);
        db.update();
      } 
      if (b.Number(kk::on_before_alarm_period, "Включение за (в секундах)")){
            addBrIn = db[kk::on_before_alarm_period].toInt32() * 1000 / 255; 
            db.update();
          }
      if (b.Number(kk::alarm_auto_off_period, "Выключение через (в секундах)")) db.update();

      if (b.Switch(kk::alarm_auto_off, "Автовыключение")) db.update();


      {
        sets::Menu m1(b, "Насторойка будильников");

        for (uint8_t i = 0; i < ALARMS_COUNT; i++) {
          if (b.beginGroup("Будильник " + String(i + 1))) {

            const String key = "alarm_data_" + String(i);

            if (b.Switch("Состояние", &((bool&)alarmLocalData[i][alarmDBData::alarm_state]))) {
              db[key] = alarmLocalData[i];
              db.update();
            }
      
            if (b.Time("Время", &alarmLocalData[i][alarmDBData::alarm_seconds])) {
              db[key] = alarmLocalData[i];
              db.update();
            }
            
            for (uint8_t j = 0; j < 7; j++) {
              if (b.Switch(daysName[j], &((bool&)alarmLocalData[i][j + 2]))) {
                db[key] = alarmLocalData[i];
                db.update();
              }
            }
            b.endGroup();
          }
        }
      }
      if (alarmControl.isStarting()) {
        if (b.Button("Выключить будильник", sets::Colors::Red)) {
          alarmControl.forceDisable();
          autoOffWaiting = false;
          ledState = false;
          b.reload();
        }
      }
    }

    {
        
        sets::Menu m2(b, "WiFi");
        b.Input(kk::wifi_ssid, "SSID");
        b.Pass(kk::wifi_pass, "Пароль");
        if (b.Button(kk::apply, "Применить")) {
            db.update();  // сохраняем БД не дожидаясь таймаута
            ESP.restart();
        }
    }
}

void update(sets::Updater& upd) {
  if (alarmControl.isStarting()) {
    upd.update("ledState"_h, ledState);
    upd.update("ledBr"_h, ledBr);
  }
}

void setup() {

    pinMode(LED_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif
    db.begin();
    db.init(kk::wifi_ssid, "");
    db.init(kk::wifi_pass, "");
    db.init(kk::time_zone, 3); // часовой пояс (по умолчанию +3)
    db.init(kk::on_before_alarm_period, 30);
    db.init(kk::alarm_auto_off_period, 15);
    db.init(kk::alarm_auto_off, true);

    // если логин задан - подключаемся
    if (db[kk::wifi_ssid].length()) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(db[kk::wifi_ssid], db[kk::wifi_pass]);
        int tries = 20;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            digitalWrite(LED_BUILTIN, 0);
            delay(50);
            digitalWrite(LED_BUILTIN, 1);
            if (!--tries) break;
        }
        if (WiFi.status() != WL_CONNECTED) startAP();
        
    }
    else startAP();

    sett.begin();
    sett.onBuild(build);
    sett.onUpdate(update);

    // будильники
    for (uint8_t i = 0; i < ALARMS_COUNT; i++) {
      const String key = "alarm_data_" + String(i);
      if (!db.has(key)) {
        const uint32_t d[alarmDBData::ITEMS_COUNT] = {
          0,
          25200, // установить семь часов утра при создании нового будильника
          0, 0, 0, 0, 0, 0, 0
        };
        db[key] = d;
      } 
      else db[key].writeTo(alarmLocalData[i]);
    }

    alarmControl.setAlarms(alarmLocalData);
    alarmControl.setAlarmPeriod(db[kk::on_before_alarm_period].toInt32());
   
    addBrIn = db[kk::on_before_alarm_period].toInt32() * 1000 / 255; 

    NTP.begin(db[kk::time_zone]);
    NTP.setPeriod(18000);
}


void loop() {
    sett.tick();
    strip.tick();
    

    if (alarmControl.isStarting()) {
      if (millis() - timeTemp >= addBrIn) {
        if (ledBr < 255) {
          ledBr++;
          strip.setBrightness(ledBr);
        }

        timeTemp = millis();
      }
    }

    if (alarmControl.isEnd()) {
        ledBr = 0;
        ledState = true;
        strip.setBrightness(ledBr);
        timeTemp = millis();
        sett.reload();
    }

    if (NTP.tick()) {
      if (alarmControl.secondTick()) {
          ledBr = 255;
          strip.setBrightness(ledBr);
          timeTemp = millis();
          autoOffWaiting = true;
          sett.reload();
      }
    }

    if (autoOffWaiting && db[kk::alarm_auto_off] && millis() - timeTemp >= db[kk::alarm_auto_off_period].toInt32()*1000) {
      ledState = false;
      ledBr = 0;
      autoOffWaiting = false;
    }

    if (alarmControl.isStarting() || autoOffWaiting) return;

    
    if (ledState) strip.setBrightness(ledBr);
    else strip.setBrightness(0);
}
