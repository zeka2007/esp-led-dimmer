# ДИММЕР ДЛЯ СВЕТОДИОДНОЙ ЛЕНТЫ НА ESP8266
Устройство для управления яркостью одноцветной светодиодной ленты/светильника с управлением через WiFi (веб-интерфейс на основе библиотеки [GyverSettings](https://github.com/GyverLibs/Settings))
## Функции
Прошивка позволяет:
- Управлять лентой как через локальную сеть WiFi, так и через свою точку доступа
- Управлять яркостью
- Управлять состоянием (включить/выключить)
- Включать ленту по будильнику (их количество меняется в прошивке)
- Автоматически выключать ленту после срабатывания будильника
## Будильник рассвет
Прошивка позволяет создать нужное количество будильников. Для каждого можно настроить время и дни недели, когда будильник будет срабатывать.
Также есть возможность настроить время за которое лента будет плавно включаться до срабатывания будильника и время автоматического выключения после срабатывания будильника.

# Установка и настройка
Управлять нагрузкой будет MOSFET, подключенный к пину D2 (меняется в прошивке) 
1. Установить [Arduion IDE](https://www.arduino.cc/en/software)
2. Настроить IDE для работы с платами esp8266
3. Установить библиотеки: [GyverNTP](https://github.com/GyverLibs/GyverNTP), [GyverSettings](https://github.com/GyverLibs/Settings)
4. Загрузить её на плату

Для управления нужно подключиться к точке доступа esp, после чего автоматически откроется страница настроек. На ней можно подключться к WiFi, после чего откроется возможность работать с платой из браузера или приложения.

**Внимание! Для работы будильника необходимо подключиться к WiFi, так как плата берет время из интернета.**