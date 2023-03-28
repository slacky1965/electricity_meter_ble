# Electricity Meter Bluetooth LE TLSR8253 (Ai-Thinker TB-04)

## Устройство для дистанционного мониторинга многотарифных однофазных счетчиков с последующей передачей показаний в Home Assistant.

**Включает в себя схему опротопорта и модуль от Ai-Thinker TB-04**

[Repository electricity_meter_ble](https://github.com/slacky1965/electricity_meter_ble)

[Схема устройства](https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/Schematic_Electricity_meter.jpg)

**Описание**

* Рассчитано на взаимодействие через оптопорт с однофазными многотарифными электросчетчиками:

> 1. [КАСКАД-1-МТ](https://github.com/slacky1965/electricity_meter_ble/tree/main/doc/electricity_meters/kaskad_1_mt#readme) (или аналогичными, где используется протокол МИРТЕК, например IEK "STAR 104/1")
> 2. [КАСКАД-11-С1](https://github.com/slacky1965/electricity_meter_ble/tree/main/doc/electricity_meters/kaskad_11_c1#readme)
> 3. [Энергомера СЕ-102М](https://github.com/slacky1965/electricity_meter_ble/tree/main/doc/electricity_meters/energomera_ce102m#readme)

В процессе работа над такими счетчиками, как Меркурий-206, Энергомера СЕ-102.

* Модуль посылает команды электросчетчику и принимает ответы от него. В настоящий момент устройство может прочитать

> 1. 3 тарифа (в kWh)
> 2. силу тока (если доступно, в A)
> 3. напряжение сети (если доступно, в V)
> 4. мощность (если доступно, в W)
> 5. оставшийся ресурс батарии прибора (если доступно, в %)
> 6. полный серийный номер прибора (например 3171112520109)
> 7. дату изготовления прибора (если доступно, например 04.10.2017)

* Сохраняет в энергонезависимой памяти модуля только конфигурационный данные.
* Передает показания по Bluetooth LE в 2-х рекламных пакетах в формате [BTHome V2.0](https://bthome.io/format/).
* Данные в пакетах могут быть зашифрованы [AES-CCM](https://bthome.io/encryption/).
* Первоначальная настройка происходит через [WEB-интерфейс](https://slacky1965.github.io/ble_utils/ElectricityMeterConfig.html). В нем же можно обновить прошивку "по воздуху".
* Соединиться с модулем можно только с того устройства, с которого производили первое соединение. Первый раз соединиться можно с любого устройства. Чтобы сбросить whitelist, нужно замкнуть контакт SWS на землю более, чем на 5 секунд (или через [WEB-интерфейс](https://slacky1965.github.io/ble_utils/ElectricityMeterConfig.html), если первоначальное устройство соединения доступно).

---

**Настройка**

Заливаем прошивку с помощью [github.com/pvvx/TLSRPGM](https://github.com/pvvx/TLSRPGM). Запускаем [WEB-конфигуратор](https://slacky1965.github.io/ble_utils/ElectricityMeterConfig.html), соединяемся с модулем, далее

> 1. Выбираем нужный счетчик из предложенных.
> 2. Устанавливаем период опроса (в минутах, от 1 до 255).
> 3. Вводим обязательно адрес счетчика (для Энергомеры СЕ-102М адрес вводить не нужно). Для КАСКАД тот, который он показывает у себя на экране, например ID-20109 (т.е. 20109). Для Меркурия это серийный номер, который напечатан на передней панели.
> 4. Если нужно шифрование передаваемых данных, вводим 16 байтный код в hex-представлении, например 231d39c1d7cc1ab1aee224cd096db932
> 5. Множитель/делитель по умолчанию подобраны под каждый счетчик. Вам нужно только проверить корректность отображения в Home Assistant и редактировать только если это необходимо (повторным соединением). Корректные значения - 0.001, 0.01, 0.1, 1, 10, 100, 1000. Умножение можно применять к power и tarrifs. Применение умножения к voltage и current может полностью изменить само значение, будьте внимательны. Деление можно применять ко всем значениям.

Отправляем настройки нажатием одноименных кнопок и все.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/images/config_html.jpg" alt="Web flasher"/>

Тарифы, мощность, напряжение и сила тока в выводе лога и в [WEB-конфигураторе](https://slacky1965.github.io/ble_utils/ElectricityMeterConfig.html) показаны в абсолютных значениях, как они хранятся в памяти.

---

**Собранное устройство**

[Печатная плата (EasyEDA)](https://oshwlab.com/slacky/electricity_meter)

Плата изначально проектировалась под вилку ~220 вольт в качестве корпуса. Позже возникли накладки по длине устройства. В общем плата осталась без изменений, но пришлось проработать дополнительный вариант корпуса, меньший по длине. Собственно вариант 1 - в качестве корпуса вилка и вариант 2 - просто коробочка, заказанная на Алиэкспресс.

[Инструкции по изготовлению готового устройства](https://github.com/slacky1965/electricity_meter_ble/tree/main/doc/device#readme)

---

Ну и напоследок скриншот из Home Assistant

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/images/home_assistant.jpg" alt="home_assistant"/>

