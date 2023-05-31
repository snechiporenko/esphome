#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace ft6336 {

typedef enum {
  FOCALTECH_EVENT_PUT_DOWN,
  FOCALTECH_EVENT_PUT_UP,
  FOCALTECH_EVENT_CONTACT,
  FOCALTECH_EVENT_NONE,
} EventFlag_t;

typedef enum {
  FOCALTECH_PMODE_ACTIVE = 0,     // ~4mA
  FOCALTECH_PMODE_MONITOR = 1,    // ~3mA
  FOCALTECH_PMODE_DEEPSLEEP = 3,  // ~100uA  The reset pin must be pulled down to wake up
} PowerMode_t;

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

 protected:
  void hard_reset_();
  bool soft_reset_();

  InternalGPIOPin *interrupt_pin_;
  GPIOPin *rts_pin_;
  FT6336TouchscreenStore store_;

 private:
  bool getPoint(uint16_t &x, uint16_t &y);
  void setTheshold(uint8_t value);
  void setPowerMode(PowerMode_t m);
};

}  // namespace ft6336
}  // namespace esphome
