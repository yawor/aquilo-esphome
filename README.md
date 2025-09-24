# Aquilo wireless water/septic tank fluid level sensor external component for ESPHome

A Polish company [Aquilo](https://aquilo.pl) produces and sells quite nicely built
[wireless fluid level sensor](https://aquilo.pl/produkt/aquilo-wifi-nadajnik-bezprzewodowy-do-czujnika-cieczy-mikrofalowy/).
To be precise, it's a microwave distance sensor, which senses the distance from itself to a fluid's surface. It can be used
for example in septic tanks or in rain water tanks.

Normally, the sensor can be bought with a Wi-Fi enabled gateway, but it can also be bought separately. The communication
between the sensor and the gateway uses LoRa modules at 868 MHz (SX1276 in the gateway, not sure about the sensor itself,
as it is hermetically sealed and I didn't want break it open). The gateway also uses ESP-12F microcontroller.

The main issue for me was that the original gateway firmware doesn't do any processing locally. It connects to Aquilo's
MQTT servers and sends the readouts. The processed data (the percentage of tank's volume occupancy) is accessible
through their mobile app and is also sent back to the gateway. The gateway has a local API, which is accessible via HTTP
requests (it is documented by Aquilo, and they also provide examples like integration in Home Assistant), but the data
is only available if the device has Internet access. The second issue is that if I would like to use multiple sensors
with a single gateway, then I would need to buy a subscription (which, to be honest, is quite cheap, but it still adds up to
all other subscriptions).

That's why I've decided to try and implement my own gateway based on ESPHome by creating this external component.

Fortunately I didn't have to implement LoRa support from the beginning. A support for SX126x and SX127x transceiver ICs has
been recently merged into the ESPHome project. Right now it's not easy to add support for both components at the same time, so
I've decided to use SX1276 transceiver in this component (and Sx127x ESPHome component).

Here's my ESPHome config, which uses [FireBeetle ESP32](https://www.dfrobot.com/product-1590.html)
with [FireBeetle LoRa 868MHz Cover](https://www.dfrobot.com/product-1831.html). I have two Aquilo sensors/transmitters which are
measuring the content levels of a septic tank and a rain water tank.

```yaml
# Some substitutions used later in the config
substitutions:
  # Names for the tanks
  septic_tank_name: "Aquilo Septic Tank"
  rain_water_tank_name: "Aquilo Rain Water Tank"

  # Tank parameters:
  # *_offset - distance of the sensor from the content surface when tank is full (100%) (in millimeters)
  # *_height - difference of surface height between full (100%) and empty tank (0%) (in millimeters)
  # *_total_volume - total tank volume (in liters)
  septic_tank_offset: "300"
  septic_tank_height: "2000"
  septic_tank_total_volume: "8000"

  rain_water_tank_offset: "165"
  rain_water_tank_height: "1240"
  rain_water_tank_total_volume: "4000"

esphome:
  name: aquilo
  comment: Aquilo Sensor Receiver

  # ESPHome supports now sub-device definition. Each sensor/tank gets its own device in Home Assistant
  devices:
    - id: septic_tank
      name: $septic_tank_name
    - id: rain_water_tank
      name: $rain_water_tank_name

  on_boot:
    then:
      # Restore distance and battery sensor states from globals on boot
      - lambda: |-
          id(septic_tank_surface_distance_sensor).publish_state(id(septic_tank_distance_store));
          id(septic_tank_battery_voltage_sensor).publish_state(id(septic_tank_voltage_store));
          id(rain_water_tank_surface_distance_sensor).publish_state(id(rain_water_tank_distance_store));
          id(rain_water_tank_battery_voltage_sensor).publish_state(id(rain_water_tank_voltage_store));

esp32:
  board: firebeetle32
  framework: 
    type: esp-idf

ota:
  - platform: esphome

api:
  encryption:
    key: !secret aquilo_encryption_key

logger:

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

external_components:
  - source: github://yawor/aquilo-esphome@main
    components: [ aquilo ]

# Globals used to store distance and battery sensor states between ESP32 restarts
globals:
  - id: septic_tank_distance_store
    type: uint16_t
    restore_value: true
  - id: septic_tank_voltage_store
    type: uint16_t
    restore_value: true
  - id: rain_water_tank_distance_store
    type: uint16_t
    restore_value: true
  - id: rain_water_tank_voltage_store
    type: uint16_t
    restore_value: true

spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19

sx127x:
  cs_pin: GPIO27
  rst_pin: GPIO26
  dio0_pin: GPIO25
  modulation: LORA
  frequency: 868000000
  bandwidth: 125_0kHz
  crc_enable: true
  spreading_factor: 12
  preamble_size: 6
  rx_start: true
  on_packet:
    then:
      # Received LoRa packet logging. Not necessary but useful for debugging or analysing local LoRa traffic.
      - lambda: |-
          ESP_LOGD("lambda", "packet %s", format_hex_pretty(x).c_str());
          ESP_LOGD("lambda", "rssi %.2f", rssi);
          ESP_LOGD("lambda", "snr %.2f", snr);

aquilo:

sensor:
  # Sensors for each transmitter
  - platform: aquilo
    transmitter_id: !secret aquilo_septic_tank_transmitter_id
    # The distance sensor for the distance measured from the transmitter to the tank content surface
    distance:
      id: septic_tank_surface_distance_sensor
      name: Raw Distance
      device_id: septic_tank
      on_value:
        then:
          # Store reported value in global variable so it's persisted between restarts
          - globals.set:
              id: septic_tank_distance_store
              value: !lambda "return x;"
    # The transmitter's battery voltage
    voltage:
      id: septic_tank_battery_voltage_sensor
      name: Battery Voltage
      device_id: septic_tank
      on_value:
        then:
          # Store reported value in global variable so it's persisted between restarts
          - globals.set:
              id: septic_tank_voltage_store
              value: !lambda "return x;"
    # The transmitter has internal counter which is incremented on each measurement. This contains current measurement number
    measurement_count:
      name: Measurement Count
      device_id: septic_tank
    # The transmitter also sends the number of the previously sent measurement. This value should be equal to previous "measurement_count".
    # If it's not equal then we've lost one or more packets.
    prev_measurement_count:
      name: Previous Measurement Count
      device_id: septic_tank
    # The transmitter's signal strength
    rssi:
      name: RSSI
      device_id: septic_tank
    # The transmitter's signal to noise ratio
    snr:
      name: SNR
      device_id: septic_tank

  - platform: aquilo
    transmitter_id: !secret aquilo_rain_water_tank_transmitter_id
    distance:
      id: rain_water_tank_surface_distance_sensor
      name: Raw Distance
      device_id: rain_water_tank
      on_value:
        then:
          - globals.set:
            # Store reported value in global variable so it's persisted between restarts
              id: rain_water_tank_distance_store
              value: !lambda "return x;"
    voltage:
      id: rain_water_tank_battery_voltage_sensor
      name: Battery Voltage
      device_id: rain_water_tank
      on_value:
        then:
          # Store reported value in global variable so it's persisted between restarts
          - globals.set:
              id: rain_water_tank_voltage_store
              value: !lambda "return x;"
    measurement_count:
      name: Measurement Count
      device_id: rain_water_tank
    prev_measurement_count:
      name: Previous Measurement Count
      device_id: rain_water_tank
    rssi:
      name: RSSI
      device_id: rain_water_tank
    snr:
      name: SNR
      device_id: rain_water_tank

  # My septic tank is a rectangular cuboid. These next two sensors copy the original distance sensor and calculate content level in % and volume in liters using lambdas and tank parameters defined earlier.
  - platform: copy
    source_id: septic_tank_surface_distance_sensor
    name: Level
    device_id: septic_tank
    device_class: ""
    unit_of_measurement: "%"
    accuracy_decimals: 1
    icon: mdi:percent
    entity_category: ""
    filters:
      - lambda: return ($septic_tank_height - (x - $septic_tank_offset)) / $septic_tank_height * 100;
      - clamp:
          min_value: 0

  - platform: copy
    source_id: septic_tank_surface_distance_sensor
    name: ""
    device_id: septic_tank
    device_class: volume_storage
    unit_of_measurement: "L"
    entity_category: ""
    accuracy_decimals: 0
    filters:
      - lambda: return ($septic_tank_height - (x - $septic_tank_offset)) / $septic_tank_height * $septic_tank_total_volume;
      - clamp:
          min_value: 0

  # My rain water tank is a horizontal cylinder with eliptical end caps. It's not easy to calculate content level in this case so this sensor uses
  # a calibrate_linear filter with datapoints taken from a TankCalc program (can be found here: https://arachnoid.com/TankCalc/index.html).
  # First we calculate the height of the water from the bottom of the tank using lambda and then we map it on the percentage level using calibrate_linear.
  - platform: copy
    id: rain_water_tank_level_sensor
    source_id: rain_water_tank_surface_distance_sensor
    name: Level
    device_id: rain_water_tank
    device_class: ""
    unit_of_measurement: "%"
    accuracy_decimals: 1
    entity_category: ""
    icon: mdi:percent
    filters:
      - lambda: return $rain_water_tank_height - (x - $rain_water_tank_offset);
      - calibrate_linear:
          method: exact
          datapoints:
            - 0 -> 0
            - 41.5 -> 1
            - 66 -> 2
            - 86.6 -> 3
            - 105.1 -> 4
            - 122.2 -> 5
            - 138.3 -> 6
            - 153.6 -> 7
            - 168.2 -> 8
            - 182.3 -> 9
            - 195.9 -> 10
            - 209.1 -> 11
            - 222 -> 12
            - 234.6 -> 13
            - 247 -> 14
            - 259.1 -> 15
            - 271 -> 16
            - 282.7 -> 17
            - 294.2 -> 18
            - 305.6 -> 19
            - 316.8 -> 20
            - 327.9 -> 21
            - 338.9 -> 22
            - 349.7 -> 23
            - 360.4 -> 24
            - 371.1 -> 25
            - 381.7 -> 26
            - 392.1 -> 27
            - 402.5 -> 28
            - 412.8 -> 29
            - 423.1 -> 30
            - 433.3 -> 31
            - 443.4 -> 32
            - 453.5 -> 33
            - 463.5 -> 34
            - 473.5 -> 35
            - 483.5 -> 36
            - 493.4 -> 37
            - 503.2 -> 38
            - 513.1 -> 39
            - 522.9 -> 40
            - 532.7 -> 41
            - 542.4 -> 42
            - 552.2 -> 43
            - 561.9 -> 44
            - 571.6 -> 45
            - 581.3 -> 46
            - 591 -> 47
            - 600.7 -> 48
            - 610.3 -> 49
            - 620 -> 50
            - 629.7 -> 51
            - 639.3 -> 52
            - 649 -> 53
            - 658.7 -> 54
            - 668.4 -> 55
            - 678.1 -> 56
            - 687.8 -> 57
            - 697.6 -> 58
            - 707.3 -> 59
            - 717.1 -> 60
            - 726.9 -> 61
            - 736.8 -> 62
            - 746.6 -> 63
            - 756.5 -> 64
            - 766.5 -> 65
            - 776.5 -> 66
            - 786.5 -> 67
            - 796.6 -> 68
            - 806.7 -> 69
            - 816.9 -> 70
            - 827.2 -> 71
            - 837.5 -> 72
            - 847.9 -> 73
            - 858.3 -> 74
            - 868.9 -> 75
            - 879.5 -> 76
            - 890.3 -> 77
            - 901.1 -> 78
            - 912.1 -> 79
            - 923.2 -> 80
            - 934.4 -> 81
            - 945.8 -> 82
            - 957.3 -> 83
            - 969 -> 84
            - 980.9 -> 85
            - 993 -> 86
            - 1005.4 -> 87
            - 1018 -> 88
            - 1030.9 -> 89
            - 1044.1 -> 90
            - 1057.7 -> 91
            - 1071.8 -> 92
            - 1086.4 -> 93
            - 1101.7 -> 94
            - 1117.8 -> 95
            - 1134.9 -> 96
            - 1153.4 -> 97
            - 1174 -> 98
            - 1198.5 -> 99
            - 1240 -> 100
      - clamp:
          min_value: 0
          max_value: 100

  # This sensor takes the percentage level of water from the previous sensor and calculates the water volume in liters.
  - platform: copy
    source_id: rain_water_tank_level_sensor
    name: ""
    device_id: rain_water_tank
    device_class: volume_storage
    unit_of_measurement: "L"
    accuracy_decimals: 0
    entity_category: ""
    icon: ""
    filters:
      - lambda: return x * $rain_water_tank_total_volume / 100;
```
