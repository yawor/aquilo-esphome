#include "aquilo_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace aquilo {

static const char *const TAG = "aquilo";

void AquiloSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "AquiloSensor:");
  LOG_SENSOR("  ", "Distance", this->distance_sensor_);
  LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("  ", "Measurement Count", this->measurement_count_sensor_);
  LOG_SENSOR("  ", "Prev Measurement Count", this->prev_measurement_count_sensor_);
  LOG_SENSOR("  ", "RSSI", this->rssi_sensor_);
  LOG_SENSOR("  ", "SNR", this->snr_sensor_);

}

void AquiloSensor::handle_message(const Message &message) {
  if (this->distance_sensor_ != nullptr)
    this->distance_sensor_->publish_state(message.distance);
  if (this->voltage_sensor_ != nullptr)
    this->voltage_sensor_->publish_state(message.voltage);
  if (this->measurement_count_sensor_ != nullptr)
    this->measurement_count_sensor_->publish_state(message.measurement_count);
  if (this->prev_measurement_count_sensor_ != nullptr)
    this->prev_measurement_count_sensor_->publish_state(message.prev_measurement_count);
  if (this->rssi_sensor_ != nullptr)
    this->rssi_sensor_->publish_state(message.rssi);
  if (this->snr_sensor_ != nullptr)
    this->snr_sensor_->publish_state(message.snr);
}

}
}
