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

Fortunately I didn't have to implement LoRa support from the beginning. There's already an [external component for
SX127x LoRa modules family](https://github.com/swoboda1337/sx127x-esphome).
There's also [an open PR](https://github.com/esphome/esphome/pull/7490) to merge it into the ESPHome.

Until the SX127x component is merged, it needs to be added as an external component as it is a required dependency of this
component. When it's merged, I'll update this README.

Here's an example of ESPHome config based on my development setup, which uses [FireBeetle ESP32](https://www.dfrobot.com/product-1590.html)
with [FireBeetle LoRa 868MHz Cover](https://www.dfrobot.com/product-1831.html):

```yaml
esphome:
  name: aquilo
  comment: Aquilo Sensor Receiver
  # Restore distance and voltage from globals into sensors.
  on_boot:
    then:
      - lambda: |-
          id(septic_tank_surface_distance_sensor).publish_state(id(septic_tank_distance_store));
          id(septic_tank_battery_voltage_sensor).publish_state(id(septic_tank_voltage_store));

esp32:
  board: firebeetle32

ota:
  - platform: esphome

api:

logger:

# Enable Wi-Fi with fallback to access point and captive portal to configure Wi-Fi connection.  
wifi:
  ap:
    ssid: "ESPHome Aquilo AP"
    password: "Change-Me"
    
captive_portal:

# Include external components.
external_components:
  - source: github://swoboda1337/sx127x-esphome@main
    components: [ sx127x ]
  - source: github://yawor/aquilo-esphome@main
    components: [ aquilo ]

# Globals storing the distance and voltage readouts from the sensor. This is needed for state persistence in case of power loses,
# as ESPHome doesn't restore sensor values by itself.
globals:
  - id: septic_tank_distance_store
    type: uint16_t
    restore_value: true
  - id: septic_tank_voltage_store
    type: uint16_t
    restore_value: true

# Configure SPI pins used to connect to SX127x module.
spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19

# Configure SX127x module parameters.
sx127x:
  cs_pin: GPIO27
  rst_pin: GPIO26
  dio0_pin: GPIO25
  modulation: LORA
  frequency: 868000000
  bandwidth: 125_0kHz
  crc_enable: true
  spreading_factor: 12
  rx_start: true
  # Extra logging from the LoRa module. Prints out packets as hex strings. Useful for debugging.
  on_packet:
    then:
      - lambda: |-
          ESP_LOGD("lambda", "packet %s", format_hex_pretty(x).c_str());
          ESP_LOGD("lambda", "rssi %.2f", rssi);
          ESP_LOGD("lambda", "snr %.2f", snr);

# Enable aquilo component
aquilo:

# Add sensors for aquilo platform. Store distance and voltage from the sensor in globals for state persistence.
sensor:
  - platform: aquilo
    # The sensor identifier as a hex number. It can be found on the sticker on the sensor itself.
    transmitter_id: 0x123456
    distance:
      id: septic_tank_surface_distance_sensor
      name: Septic Tank Surface Distance
      on_value:
        then:
          - globals.set:
              id: septic_tank_distance_store
              value: !lambda "return x;"
    voltage:
      id: septic_tank_battery_voltage_sensor
      name: Septic Tank Battery Voltage
      on_value:
        then:
          - globals.set:
              id: septic_tank_voltage_store
              value: !lambda "return x;"
    measurement_count:
      name: Septic Tank Measurement Count
    prev_measurement_count:
      name: Septic Tank Previous Measurement Count
    rssi:
      name: Septic Tank RSSI
    snr:
      name: Septic Tank SNR
```

I think it also should be possible to reflash the ESP-12F in original Aquilo Wi-Fi gateway, but I haven't tested that yet.
If I do then I'll update the README and include the ESPHome configuration for it.