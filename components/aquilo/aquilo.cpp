#include "aquilo.h"
#include "esphome/core/log.h"

namespace esphome {
namespace aquilo {

static const char *const TAG = "aquilo";

void Aquilo::loop() {
  if (!this->messages_.empty()) {
    Message message = this->messages_.front();
    this->messages_.pop();

    ESP_LOGI(TAG, "Received message from %x: distance %d mm, voltage %d mV, measurement %d (prev %d)", message.transmitter_id, message.distance, message.voltage, message.measurement_count, message.prev_measurement_count);

    for (auto &listener : this->listeners_)
      listener->on_message(message);
  } else if (!this->packets_.empty()) {
    Packet packet = this->packets_.front();
    this->packets_.pop();

    ESP_LOGI(TAG, "Received packet: %s, RSSI: %f, SNR: %f", packet.content.c_str(), packet.rssi, packet.snr);

    uint32_t transmitter_id, measurement_count, prev_measurement_count;
    uint16_t distance, voltage;
    if (sscanf(packet.content.c_str(), "Uf_%*[^_]_%lx_%hu_%hu_%*d_%*d_%u_%u_%*d_%*d", &transmitter_id, &distance, &voltage, &measurement_count, &prev_measurement_count) != 5)
      return;

    this->messages_.push({transmitter_id, measurement_count, prev_measurement_count, distance, voltage, packet.rssi, packet.snr});
  }
}

void Aquilo::on_packet(const std::vector<uint8_t> &packet, float rssi, float snr) {
  if (packet.size() < 3 || packet[0] != 'U' || packet[1] != 'f' || packet[2] != '_')
    return;

  const std::string content(packet.begin(), packet.end());
  this->packets_.push({content, rssi, snr});
}

}  // namespace aquilo
}  // namespace esphome

