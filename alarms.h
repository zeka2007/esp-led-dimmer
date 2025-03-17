#include <GyverNTP.h>
#include <Arduino.h>

class Alarm {
    public:
        Alarm (GyverNTP *NTP) {
            _NTP = NTP;
        };

        bool secondTick();
        bool isStarting();
        bool isEnd();

        void forceDisable() {
            _alarmStarting = false;
        };

        void setAlarms (uint32_t (*alarmData)[alarmDBData::ITEMS_COUNT]) {
            _alarmData = alarmData;
        };

        void setAlarmPeriod (uint32_t period) {
            _alarmPeriod = period;
        };

    private:
        GyverNTP *_NTP;
        uint32_t (*_alarmData)[alarmDBData::ITEMS_COUNT];

        bool _alarmStarting = false;
        bool _isEndFlag = false;
        uint8_t _alarmStartingId = 0;
        uint32_t _alarmPeriod = 0;
        uint32_t _timeStap;
};


bool Alarm::isStarting() {
    return _alarmStarting;
};

bool Alarm::isEnd() {
    if (_isEndFlag) {
        _isEndFlag = false;
        return true;
    }
    return false;
};

bool Alarm::secondTick() {
    if (_alarmStarting) {
        Datime dt(*_NTP);
        if (_alarmData[_alarmStartingId][alarmDBData::alarm_seconds] == dt.daySeconds()) {
            _alarmStarting = false;
            _isEndFlag = true;
            return true;
        }
    }
    else {
        for (uint8_t i = 0; i < ALARMS_COUNT; i++) {
            if (_alarmData[i][alarmDBData::alarm_state]) {
                Datime dt(*_NTP);
                if (_alarmData[i][dt.weekDay + 1]) {

                    Datime alarmDT(*_NTP);

                    if(_alarmData[i][alarmDBData::alarm_seconds] - dt.daySeconds() == _alarmPeriod) {
                        _alarmStartingId = i;
                        _alarmStarting = true;
                        _isEndFlag = false;
                    }
                }   
            }
        }
    }
    return false;
};