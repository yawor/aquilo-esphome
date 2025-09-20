#include "aquilo.h"
#include "esphome/core/log.h"

namespace esphome {
namespace aquilo {

static const char *const TAG = "aquilo";

void Aquilo::process_packet_(const std::vector<uint8_t> &packet, float rssi, float snr) {
  const std::string content(packet.begin(), packet.end());

  ESP_LOGI(TAG, "Received packet: %s, RSSI: %f, SNR: %f", content.c_str(), rssi, snr);

  uint32_t transmitter_id, measurement_count, prev_measurement_count;
  uint16_t distance, voltage;
  if (sscanf(content.c_str(), "Uf_%*[^_]_%lx_%hu_%hu_%*d_%*d_%u_%u_%*d_%*d", &transmitter_id, &distance, &voltage, &measurement_count, &prev_measurement_count) != 5)
    return;

  const Message message{transmitter_id, measurement_count, prev_measurement_count, distance, voltage, rssi, snr};

  ESP_LOGI(TAG, "Received message from %x: distance %d mm, voltage %d mV, measurement %d (prev %d)", message.transmitter_id, message.distance, message.voltage, message.measurement_count, message.prev_measurement_count);

  for (auto &listener : this->listeners_)
    this->defer([listener, message] { listener->on_message(message); });
 }

void Aquilo::on_packet(const std::vector<uint8_t> &packet, float rssi, float snr) {
  if (packet.size() < 3 || packet[0] != 'U' || packet[1] != 'f' || packet[2] != '_')
    return;
  
  this->defer([this, packet, rssi, snr] { this->process_packet_(packet, rssi, snr); });
}

}  // namespace aquilo
}  // namespace esphome

