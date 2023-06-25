#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "focaltech.h"

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
  static FT6336Touchscreen *instance;
  FT6336Touchscreen();

  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_rts_pin(GPIOPin *pin) { this->rts_pin_ = pin; }
  void active(bool is_active);

 protected:
  void hard_reset_();
  InternalGPIOPin *interrupt_pin_;
  GPIOPin *rts_pin_;
  FT6336TouchscreenStore store_;
  FocalTech_Class *focaltech = new FocalTech_Class();

  static uint8_t read_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
  static uint8_t write_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
};

}  // namespace ft6336
}  // namespace esphome
