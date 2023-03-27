# Однофазный многотарифный счетчик КАСКАД-1-МТ

<img src="https://raw.githubusercontent.com/slacky1965/electricity_meter_ble/main/doc/electricity_meters/kaskad_1_mt/kaskad_1_mt.jpg" alt="KASKAD-1-MT">

## Описание особенностей работы с этим счетчиком

Счетчик общается по протоколу [МИРТЕК](https://github.com/slacky1965/electricity_meter_ble/blob/main/doc/electricity_meters/kaskad_1_mt/Star_104_304_1.20.doc?raw=true).

В настоящий момент устройство может прочитать из счетчика:

> 1. 3 тарифа (в kWh)
> 2. силу тока (в A)
> 3. напряжение сети (в V)
> 4. мощность (в W)
> 5. оставшийся ресурс батарии прибора (в %)
> 6. полный серийный номер прибора (например 3171112520109)
> 7. дату изготовления прибора (например 04.10.2017)