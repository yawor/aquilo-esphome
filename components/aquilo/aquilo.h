#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sx127x/sx127x.h"

namespace esphome {
namespace aquilo {

struct Message {
 uint32_t transmitter_id;
 uint32_t measurement_count;
 uint32_t prev_measurement_count;
 uint16_t distance;
 uint16_t voltage;
 float rssi;
 float snr;
};

class AquiloListener {
 public:
  void set_transmitter_id(uint32_t transmitter_id) { this->transmitter_id_ = transmitter_id; }

  void on_message(const Message &message) { if (this->transmitter_id_ == message.transmitter_id) this->handle_message(message); }

 protected:
  uint32_t transmitter_id_{0};

  virtual void handle_message(const Message &message) = 0;
};

class Aquilo : public sx127x::SX127xListener, public Component {
 public:
  //void dump_config() override;

  void on_packet(const std::vector<uint8_t> &packet, float rssi, float snr) override;

  void register_listener(AquiloListener *listener) { this->listeners_.push_back(listener); }

 protected:
  void process_packet_(const std::vector<uint8_t> &packet, float rssi, float snr);

  std::vector<AquiloListener *> listeners_{};
};

}  //namespace aquilo
}  //namespace esphome

