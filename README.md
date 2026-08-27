# STM32 SPE MQTT Sensor Node

Firmware from an internship project: a 10BASE-T1L (Single Pair Ethernet) sensor node
on evaluation hardware.

An STM32 Nucleo-L496ZG-P communicates with an Analog Devices CN0575
(ADIN1110 MAC-PHY) over Open Alliance SPI, samples three sensors and publishes
minimum, maximum and average values via MQTT over SPE. Two configurations exist:
MQTT with TLS (mbedTLS) and MQTT without TLS. The CN0575 is supplied over the
same pair using PoDL Class 11.

This repository is a firmware snapshot of that test setup, not a product tree.

## Goal

Demonstrate a complete path on a single twisted pair:

`sensor → FreeRTOS → LwIP → MQTT → ADIN1110 → SPE`

and keep a stable link on ordinary two-wire field cable, not only on a dedicated
SPE patch cord. The work continued an existing SPE bring-up and added MQTT, TLS,
sensor processing and cable measurements.

## Sensors

| Sensor  | Interface      | Period | Data                     |
| ------- | -------------- | ------ | ------------------------ |
| ADXL345 | I2C `0x53`     | 10 ms  | X / Y / Z acceleration   |
| TMP102  | I2C `0x48`     | 5 s    | Temperature (°C)         |
| ACS723  | ADC1 CH8 (PA3) | 5 s    | Current                  |

FreeRTOS tasks write into a shared `latestSensorData` structure protected by a
mutex. `SensorDataMQTTTask` publishes every 20 s (MQTT QoS 1, keep-alive 60 s).

## Hardware

- STM32 Nucleo-L496ZG-P (STM32L496ZGT6P, 80 MHz)
- Analog Devices CN0575 (ADIN1110 10BASE-T1L MAC-PHY, PoDL powered device)
- Lab SPE switch and MQTT broker (not included in this repository)

ADIN1110 interface: SPI1, Mode 3, 20 MHz, software chip-select on PD14,
64-byte Open Alliance chunks, cut-through MAC mode.

## Software

- C, STM32 HAL, STM32CubeIDE (`ADIN1110.ioc`)
- FreeRTOS 10.3.1 (CMSIS-RTOS v2, 1 ms tick)
- LwIP 2.0.2 (DHCP, TCP, SNTP, MQTT client)
- mbedTLS 2.16.12 (TLS configuration only)

| Task                     | Priority | Period | Role              |
| ------------------------ | -------- | ------ | ----------------- |
| `NetworkMaintenanceTask` | Realtime | 1 ms   | LwIP and ADIN1110 |
| `SensorDataMQTTTask`     | High     | 20 s   | MQTT publish      |
| `AccelTask`              | Normal   | 10 ms  | ADXL345           |
| `TempTask`               | Low      | 5 s    | TMP102            |
| `ADCTask`                | Low      | 5 s    | ACS723            |

A Django page in the lab plotted data from the broker. That application is not
part of this repository.

## Repository layout

Default branch: `MQTT_TLS`.

| Path                                | Contents                          |
| ----------------------------------- | --------------------------------- |
| `Core/`                             | Application code                  |
| `Drivers/`                          | STM32L4 HAL                       |
| `MQTT TLS/`                         | TLS configuration / build         |
| `MQTT_NO_TLS/`                      | Configuration without TLS         |
| `lwip/`                             | LwIP stack                        |
| `mbedtls-2.16.12/`                  | mbedTLS                           |
| `Middlewares/Third_Party/FreeRTOS/` | FreeRTOS                          |
| `STATS/`                            | PHY / link-quality helpers        |
| `Layer_2 Mode/`                     | Earlier layer-2 bring-up          |
| `ADIN1110.ioc`                      | CubeMX configuration              |

## Build

1. Install STM32CubeIDE.
2. Check out the `MQTT_TLS` branch.
3. Open the project or `ADIN1110.ioc`.
4. Build the **MQTT TLS** or **MQTT_NO_TLS** configuration.
5. Flash the Nucleo-L496ZG-P.

## Measurements

Link tests on several two-wire / field cables at 1 m, 10 m, 50 m and 100 m
(including unshielded sensor cable and Etherline T11 Flex):

- 10 000 ICMP echoes, 0 % loss
- PHY link state Good, SQI 7
- MSE approximately −30 dB, SNR approximately 30 dB
- No RX CRC or PHY errors in those runs
- SPE port power about 0.67–0.71 W; only a small voltage drop at the powered
  device over 100 m

## License

No separate license is published for the internship application code.

Third-party components remain under their own licenses:

- STMicroelectronics STM32 HAL
- LwIP
- FreeRTOS
- ARM mbedTLS
