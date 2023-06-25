#ifndef __AXP202_H__
#define __AXP202_H__

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "axp20x.h"

namespace esphome {
namespace axp202 {

struct AXP202ComponentStore {
  volatile bool click;
  ISRInternalGPIOPin pin;

  static void gpio_intr(AXP202ComponentStore *store);
};

class AXP202Component : public PollingComponent, public i2c::I2CDevice {
 public:
  static AXP202Component *instance;
  AXP202Component();

  void set_batterylevel_sensor(sensor::Sensor *batterylevel_sensor) { batterylevel_sensor_ = batterylevel_sensor; }
  void set_brightness(float brightness) { brightness_ = brightness; }

  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  void setup() override;
  void loop() override;
  void dump_config() override;
  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  float get_setup_priority() const override;
  void update() override;

  /** Add a callback to be notified of state changes.
   *
   * @param callback The void(bool) callback.
   */
  void add_on_click_callback(std::function<void()> &&callback);

  static void handleClick();

  int getBattPercentage();
  void setPowerOutPut(uint8_t ch, bool en);
  bool isChargeing();
  bool isEnable(uint8_t ch);

 protected:
  AXP20X_Class *axp20x = new AXP20X_Class();
  sensor::Sensor *batterylevel_sensor_;
  InternalGPIOPin *interrupt_pin_;
  AXP202ComponentStore store_;
  float brightness_{1.0f};
  float curr_brightness_{-1.0f};
  CallbackManager<void()> click_callback_{};

  static uint8_t read_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
  static uint8_t write_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
};

class ClickTrigger : public Trigger<> {
 public:
  explicit ClickTrigger(AXP202Component *parent) {
    parent->add_on_click_callback([this]() { this->trigger(); });
  }
};

}  // namespace axp202
}  // namespace esphome

#endif
