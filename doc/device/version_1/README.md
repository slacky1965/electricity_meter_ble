# Electricity Meter Bluetooth LE TLSR8253 (Ai-Thinker TB-04)

## Небольшая инструкция, как сделать устройство для дистанционного мониторинга многотарифного однофазного счетчика КАСКАД-1-МТ с последующей передачей показаний в Home Assistant.

**Включает в себя печатную плату и разборную вилку на ~200V**

В данном проекте корпус сделан из вилки "Lezard" на ~220 вольт из магазина Луруа Мерлен. 

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/new_fork.jpg" alt="New Fork">

Берем вилку, откручиваем крепежный винт, вынимаем из корпуса вилки внутренности.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/inside_fork_1.jpg" alt="Fork inside1">

Далее откручиваем все крепежи, вынимаем силовые штыри (они вытаскиваются вперед почти без усилия). Далее пассатижами в небольшим усилием "отрываем" земляную клемму. В конце у вас на внутренней част вилки не должно остаться ни одной металлической детали, только платик.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/inside_fork_2.jpg" alt="Fork inside2">

Далее удаляем внутренние посадочные места для крепления силовых штырей.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/inside_fork_3.jpg" alt="Fork inside3">

И сверлом расширяем "окошко" для светодиодов, удалив перемычку.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/fork_inside_window.jpg" alt="Fork inside window">

Крепим готовую плату через изоляционные шайбы (можно использовать фиксатор для провода от этой же вилки, только откусить кусачками лишнее). При роектировании платы была допущена небольшая неточность в размерах, крепежные отверстия на плате получились немного сдвинуты вперед на какую-то долю мм. Поэтому пришлось немного подрезать центральный "столбик".

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/board_top_fixing.jpg" alt="Board fixing">
