#pragma once

#include "esphome/components/aquilo/aquilo.h"
#include "esphome/core/component.h"

namespace esphome {
namespace aquilo {

class AquiloSensor : public AquiloListener, public Component {
 public:
  void dump_config() override;
  void set_distance_sensor(sensor::Sensor *sensor) { this->distance_sensor_ = sensor; }
  void set_voltage_sensor(sensor::Sensor *sensor) { this->voltage_sensor_ = sensor; }
  void set_measurement_count_sensor(sensor::Sensor *sensor) { this->measurement_count_sensor_ = sensor; }
  void set_prev_measurement_count_sensor(sensor::Sensor *sensor) { this->prev_measurement_count_sensor_ = sensor; }
  void set_rssi_sensor(sensor::Sensor *sensor) { this->rssi_sensor_ = sensor; }
  void set_snr_sensor(sensor::Sensor *sensor) { this->snr_sensor_ = sensor; }

 protected:
  sensor::Sensor *distance_sensor_{nullptr};
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *measurement_count_sensor_{nullptr};
  sensor::Sensor *prev_measurement_count_sensor_{nullptr};
  sensor::Sensor *rssi_sensor_{nullptr};
  sensor::Sensor *snr_sensor_{nullptr};

  void handle_message(const Message &message) override;
};

} // namespace aquilo
} // namespace esphome
