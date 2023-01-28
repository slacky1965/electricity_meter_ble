# Electricity Meter Bluetooth LE TLSR8253 (Ai-Thinker TB-04)

##Устройство для дистанционного мониторинга многотарифного однофазного счетчика КАСКАД-1-МТ с последующей передачей показаний в Home Assistant.

**Включает в себя схему опротопорта и модуль от Ai-Thinker TB-04**

[Repository electricity_meter_ble](https://github.com/slacky1965/electricity_meter_ble)

[Схема устройства](https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/Schematic_Electricity_meter.jpg)

**Принцип работы**

Модуль TB-04, соединенный по UART со схемой оптопорта, посылает команды электросчетчику и принимает ответы от него. Интерпретирует ответы и передает по Bluetooth LE в Home Assistant. На текущий момент устройство может прочитать 3 тарифа в kWh, силу тока в A, напряжение сети в V, мощность в W.

**Настройка**

Заливаем прошивку с помощью [github.com/pvvx/TLSRPGM](https://github.com/pvvx/TLSRPGM). Запускаем конфигуратор, соединяемся с модулем, вводим период опроса (в минутах), и обязательно адрес счетчика, который он показывает у себя на экране. Отправляем настройки нажатием одноименных кнопок и все.

**Собранное устройство**

Собранного законченного устройства пока нет. Сделано и проверено все на макетке. Но все работает. Повторить сможет даже школьник. Удачи.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/images/config_html.jpg" alt="Web flasher"/>

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/images/optoport_maket.jpg" alt="maket1"/>

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/images/optoport_TB4kit.jpg" alt="maket2"/>

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/images/kaskad-1-mt.jpg" alt="kackad-1-mt"/>

