# ESP-IDF Sensor monitor module

| task                   | priority | comments | done | tested |
| ---------------------- | -------- | -------- | ---- | ------ |
| OLED Screen            | 0        |          | Y    | Y      |
| Wifi                   | 0        |          | Y    | Y      |
| Wifi/BT local settings | 0        |          |      |        |
| Ethernet               | 4        |          | Y    | Y      |
| SHT31                  | 0        |          | Y    | Y      |
| D7s                    | 10       |          |      |        |
| Sensor API             | 10       |          | Y    | Y      |
| MQTT init              | 2        |          | Y    | Y      |
| MQTT used              | 3        |          | Y    | Y      |
| MQTT secured           | 10       |          |      |        |
| SPIFFS                 | 0        |          | Y    | Y      |
| LORA                   | 0        |          |      |        |
| Read local config      | 0        |          | Y    | Y      |
| Save local config      | 10       |          |      |        |
| Use local config       | 10       |          | Y    | Y      |
| WebServer settings     | 10       |          |      |        |
| WebServer/remote log   | 10       |          |      |        |
| Webserver display      | 10       |          |      |        |
| OTA update             | 10       |          |      |        |

## WESP32:

I2C1 SCL - IO5

I2C1 SDA - IO33

## STH31

STH31 I2C address = 0x44

## D7s

D7s I2C address = 0x55

D7s INT1 = IO34

D7s INT2 = IO36

https://omronfs.omron.com/en_US/ecb/products/pdf/en-d7s.pdf

https://www.futurashop.it/image/catalog/data/Download/7100-BREAKOUT019/D7S.pdf

https://github.com/alessandro1105/D7S_Arduino_Library

https://www.14core.com/wiring-d7s-seismic-earthquake-detection-mapping-intensity-vibration-sensor-on-microcontroller/

---

Based on the template application to be used with [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).

Please check [ESP-IDF docs](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for getting started instructions.

_Code in this repository is in the Public Domain (or CC0 licensed, at your option.)
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied._
