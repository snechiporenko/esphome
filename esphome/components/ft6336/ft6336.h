#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace ft6336 {

struct FT6336TouchscreenStore {
  volatile bool touch;
  ISRInternalGPIOPin pin;

  static void gpio_intr(FT6336TouchscreenStore *store);
};

using namespace touchscreen;

class FT6336Touchscreen : public Touchscreen, public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_rts_pin(GPIOPin *pin) { this->rts_pin_ = pin; }

  void set_power_state(bool enable);
  bool get_power_state();

 protected:
  void hard_reset_();
  bool soft_reset_();

  InternalGPIOPin *interrupt_pin_;
  GPIOPin *rts_pin_;
  FT6336TouchscreenStore store_;
  uint16_t x_resolution_;
  uint16_t y_resolution_;
};

}  // namespace ft6336
}  // namespace esphome
