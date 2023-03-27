# Electricity Meter Bluetooth LE TLSR8253 (Ai-Thinker TB-04)

## Небольшая инструкция, как сделать устройство для дистанционного мониторинга многотарифного однофазного счетчика КАСКАД-1-МТ с последующей передачей показаний в Home Assistant.

**Включает в себя [печатную плату](https://oshwlab.com/slacky/electricity_meter) и разборную вилку на ~200V**

В данном проекте корпус сделан из вилки "Lezard" на ~220 вольт из магазина Луруа Мерлен. 

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/new_fork.jpg" alt="New Fork">

Берем вилку, откручиваем крепежный винт, вынимаем из корпуса вилки внутренности.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/inside_fork_1.jpg" alt="Fork inside1">

Далее откручиваем все крепежи, вынимаем силовые штыри (они вытаскиваются вперед почти без усилия). Далее пассатижами в небольшим усилием "отрываем" земляную клемму. В конце у вас на внутренней части вилки не должно остаться ни одной металлической детали, только платик.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/inside_fork_2.jpg" alt="Fork inside2">

Далее удаляем внутренние посадочные места для крепления силовых штырей.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/inside_fork_3.jpg" alt="Fork inside3">

И сверлом расширяем "окошко" для светодиодов, удалив перемычку.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/fork_inside_window.jpg" alt="Fork inside window">

Крепим готовую плату через изоляционные шайбы (можно использовать фиксатор для провода от этой же вилки, только откусить кусачками лишнее). При роектировании платы была допущена небольшая неточность в размерах, крепежные отверстия на плате получились немного сдвинуты вперед на какую-то долю мм. Поэтому пришлось немного подрезать центральный "столбик". Да, еще. Гребенку J1 лучше припаять с другой стороны, место там есть. А с этой стороны пины пришлось укорачивать на половину.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/board_top_fixing.jpg" alt="Board fixing">

Далее нужно найти пластиковую прозрачную коробку от CD-диска. Из нее мы будем делать экран, чтобы закрыть отверстие со светодиодами и чтобы в дальнейшем приклеить на него магнит. Аккуратно выкусываем немного больший, чем нужно по размеру, кусок из коробки от CD-диска и приклеиваем его к площадке со стороны светодиодов.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/inside_fork_bord_screen.jpg" alt="Fork inside screen">

После того, как клей хорошо схватится, собираем вилку. И аккуратно обкусываем экран по контуру (маленькими кусочками, чтобы избежать трещин) и доводим надфилем.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/fork_screen.jpg" alt="Fork screen">

Осталось приклеить к экрану магнит. Я применил вот такой - [Неодимовый магнит кольцо 20х16х1.25 мм](https://mirmagnitov.ru/product/nyeodimovyy-magnit-kol-tso-20kh16kh1-25-mm/)

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/fork_screen_magnet.jpg" alt="Fork screen magnet">

Ну и вот так это выглядит в реальном электрощитке.

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/device/version_1/images/device_install.jpg" alt="Device install">

Питание берется с розетки на DIN-рейку через зарядку от мобильного телефона.

